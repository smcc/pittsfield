SECTION:=--section-start .text=0x90000000 --section-start .data=0x40000000
#SECTION:=--section-start .text=0x10000000 --section-start .data=0x20000000

VERSION:=0.2

OPT:=-O3 -ffast-math
DEBUG:=-g
CC:=gcc
CXX:=g++ -fno-exceptions -fno-rtti
AS:=as
TFF:=./topformflat

.PRECIOUS: %.o %.s %.fis %.fio %-raw %-noebx %-pad %-pad-noebx

loader:	loader.c wrappers.h sizes.h high-link.x
	$(CC) -Wall -g -static loader.c -lelf -lm -Wl,-T -Wl,high-link.x -o loader
	@#$(CC) -Wall -g loader.c -lelf -lm -o loader

wrappers.h: gen-stubs
stub-list: gen-stubs
gen-stubs: stubs.h
	perl index-stubs.pl stubs.h
	touch $@

sizes.h: sizes.pm
	perl -Msizes -e 'sizes::write_header' >sizes.h

%.s:	%.c libc.h stub-list sizes.h
	$(CC) -Wall -S $(DEBUG) $(OPT) --fixed-ebx $*.c

%.s:	%.cc libc.h stub-list sizes.h
	$(CXX) -Wall -S $(DEBUG) $(OPT) --fixed-ebx $*.cc

gcc-mod.s:	gcc-mod-fewer-lines.c libc.h stub-list sizes.h
	$(CC) -Wall -S $(DEBUG) $(OPT) --fixed-ebx gcc-mod-fewer-lines.c -o $@

%-fewer-lines.c:	%.c sizes.h libc.c libc.h stubs.h
	$(CC) -E $*.c | $(TFF) 0 | perl -ne 'print unless /^# / or /^\s*$$/' >$*-fewer-lines.c

%-no-stubs.c:	%.c libc.h
	$(CC) -DNO_STUBS -E $*.c | $(TFF) 0 | perl -ne 'print unless /^# / or /^\s*$$/' >$*-no-stubs.c

%-no-stubs.cc:	%.cc libc.h
	$(CXX) -DNO_STUBS -E $*.cc | $(TFF) 0 | perl -ne 'print unless /^# / or /^\s*$$/' >$*-no-stubs.cc

libc.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main libc.s >$@

libc-no-sfi-base.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -no-rodata-only libc.s >$@

libc-no-stubs.o:	libc.c libc.h
	$(CC) $(OPT) -DNO_STUBS -c $< -o $@

libcplusplus-no-sfi-base.fis:	libcplusplus.s rewrite.pl x86_common.pm sizes.pm
	perl rewrite.pl -no-rodata-only libcplusplus.s >$@


%.fis:	%.s rewrite.pl rewrite-stringops.pl x86_common.pm sizes.pm stub-list
	perl rewrite-stringops.pl $*.s >$*-nstr.s
	perl rewrite.pl $*-nstr.s >$*.fis

%.mo:	%.fis
	$(AS) $*.fis -o $*.mo

crtbegin.o: crtbegin.S
	$(AS) crtbegin.S -o crtbegin.o

crtend.o: crtend.S
	$(AS) crtend.S -o crtend.o

%-cpp-mod.fio:	%-cpp-mod.mo libc.mo crtbegin.o crtend.o libcplusplus.mo link-c++.x
	ld $(SECTION) -T link-c++.x crtbegin.o libc.mo libcplusplus.mo $*-cpp-mod.mo crtend.o -o $*-cpp-mod.fio

%.fio:	%.mo libc.mo
	ld $(SECTION) libc.mo $*.mo -o $*.fio

%.check: %.fio verify.pl x86_common.pm sizes.pm
	objdump -d $*.fio | perl verify.pl 2>&1 | tee $*.check

%-no-stubs-ebx.s:	%-no-stubs.c
	$(CC) $(OPT) -S $*-no-stubs.c -o $*-no-stubs-ebx.s

%-no-stubs-ebx.s:	%-no-stubs.cc
	$(CXX) $(OPT) -S $*-no-stubs.cc -o $*-no-stubs-ebx.s

%-no-ebx.s:	%-no-stubs.c 
	$(CC) $(OPT) -S --fixed-ebx $*-no-stubs.c -o $*-no-ebx.s

%-no-ebx.s:	%-no-stubs.cc 
	$(CXX) $(OPT) -S --fixed-ebx $*-no-stubs.cc -o $*-no-ebx.s

%-raw:	%-no-stubs-ebx.s libc-no-stubs.o outside.c
	$(CC) $(OPT) $*-no-stubs-ebx.s libc-no-stubs.o outside.c -o $*-raw -lm

%-noebx:	%-no-ebx.s libc-no-stubs.o outside.c
	$(CC) $(OPT) $*-no-ebx.s libc-no-stubs.o outside.c -o $*-noebx -lm

%-pad.s:	%-no-stubs-ebx.s rewrite.pl
	perl rewrite.pl -padonly $*-no-stubs-ebx.s >$*-pad.s

%-pad:	%-pad.s libc-no-stubs.o outside.c
	$(CC) $*-pad.s libc-no-stubs.o outside.c -o $*-pad -lm

%-pad-noebx.s:	%-no-ebx.s rewrite.pl
	perl rewrite.pl -padonly $*-no-ebx.s >$*-pad-noebx.s

%-pad-noebx:	%-pad-noebx.s libc-no-stubs.o outside.c pad.pl
	$(CC) $*-pad-noebx.s libc-no-stubs.o outside.c -o $*-pad-noebx -lm

%.out:	%.fio %-raw %-noebx %-pad %-pad-noebx loader
	-/usr/bin/time -f '%e %U %S' -o $*.out ./$*-raw 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-noebx 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-pad 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-pad-noebx 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./loader $*.fio

dist:
	rm -rf pittsfield-$(VERSION)
	mkdir pittsfield-$(VERSION)
	cp -p `awk '{print $$1}' MANIFEST` pittsfield-$(VERSION)
	tar cvzf pittsfield-$(VERSION).tar.gz pittsfield-$(VERSION)
