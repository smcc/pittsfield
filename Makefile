SECTION:=--section-start .text=0x90000000 --section-start .data=0x40000000
#SECTION:=--section-start .text=0x10000000 --section-start .data=0x20000000

VERSION:=0.2

OPT:=-O3 -ffast-math
DEBUG:=-g
CC:=gcc
CXX:=g++ -fno-exceptions -fno-rtti
AS:=as
TFF:=./topformflat
NO_EBX := --fixed-ebx
NO_SCHD := -fno-schedule-insns2

.PRECIOUS: %.o %.s %.fis %.fio %-raw %-noebx %-pad %-pad-noebx

loader:	loader.c wrappers.h sizes.h high-link.x
	$(CC) -Wall -DVERIFY -g -static loader.c libdisasm.a -lelf -lm -Wl,-T -Wl,high-link.x -o loader
	@#$(CC) -Wall -g loader.c -lelf -lm -o loader

wrappers.h: gen-stubs
stub-list: gen-stubs
gen-stubs: stubs.h
	perl index-stubs.pl stubs.h
	touch $@

sizes.h: sizes.pm
	perl -Msizes -e 'sizes::write_header' >sizes.h

%.s:	%.c libc.h stub-list sizes.h
	$(CC) -Wall -S $(DEBUG) $(OPT) $(NO_SCHD) $(NO_EBX) $*.c

%-ebx.s:	%.c libc.h stub-list sizes.h
	$(CC) -Wall -S $(DEBUG) $(OPT) $(NO_SCHD) $*.c -o $@

%-ebx-schd.s:	%.c libc.h stub-list sizes.h
	$(CC) -Wall -S $(DEBUG) $(OPT) $*.c -o $@

%.s:	%.cc libc.h stub-list sizes.h
	$(CXX) -Wall -S $(DEBUG) $(OPT) $(NO_SCHD) $(NO_EBX) $*.cc

%-ebx.s:	%.cc libc.h stub-list sizes.h
	$(CXX) -Wall -S $(DEBUG) $(OPT) $(NO_SCHD) $*.cc -o $@

%-ebx-schd.s:	%.cc libc.h stub-list sizes.h
	$(CXX) -Wall -S $(DEBUG) $(OPT) $*.cc -o $@

gcc-mod.s:	gcc-mod-fewer-lines.c libc.h stub-list sizes.h
	$(CC) -Wall -S $(DEBUG) $(OPT) $(NO_EBX) gcc-mod-fewer-lines.c -o $@

%-fewer-lines.c:	%.c sizes.h libc.c libc.h stubs.h
	$(CC) -E $*.c | $(TFF) 0 | perl -ne 'print unless /^# / or /^\s*$$/' >$*-fewer-lines.c

%-no-stubs.c:	%.c libc.h
	$(CC) -DNO_STUBS -E $*.c | $(TFF) 0 | perl -ne 'print unless /^# / or /^\s*$$/' >$*-no-stubs.c

%-no-stubs.cc:	%.cc libc.h
	$(CXX) -DNO_STUBS -E $*.cc | $(TFF) 0 | perl -ne 'print unless /^# / or /^\s*$$/' >$*-no-stubs.cc

# libc{,plusplus}{,-{jo,noop,pushf,no-sfi-{base,noschd,noebx,pad,{noop,pushf}{,-jo}}}}.mo 

libc.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main libc.s >$@

libc-noop.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -nop-only libc.s >$@

libc-pushf.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -pushf-and-nop libc.s >$@

libc-jo.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -jump-only libc.s >$@

libc-no-sfi-base.fis:	libc-ebx-schd.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -no-rodata-only libc-ebx-schd.s >$@

libc-no-sfi-noschd.fis:	libc-ebx.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -no-rodata-only libc-ebx.s >$@

libc-no-sfi-noebx.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -no-rodata-only libc.s >$@

libc-no-sfi-pad.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -no-sand libc.s >$@

libc-no-sfi-noop.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -nop-only libc.s >$@

libc-no-sfi-noop-jo.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -nop-only -jump-only libc.s >$@

libc-no-sfi-pushf.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -pushf-and-nop libc.s >$@

libc-no-sfi-pushf-jo.fis:	libc.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -main -pushf-and-nop -jump-only libc.s >$@

libc-no-stubs.o:	libc.c libc.h
	$(CC) $(OPT) -DNO_STUBS -c $< -o $@

libcplusplus-no-sfi-base.fis:	libcplusplus-ebx-schd.s rewrite.pl x86_common.pm sizes.pm
	perl rewrite.pl -no-rodata-only libcplusplus-ebx-schd.s >$@

libcplusplus-no-sfi-noschd.fis:	libcplusplus-ebx.s rewrite.pl x86_common.pm sizes.pm
	perl rewrite.pl -no-rodata-only libcplusplus-ebx.s >$@

libcplusplus-no-sfi-noebx.fis:	libcplusplus.s rewrite.pl x86_common.pm sizes.pm
	perl rewrite.pl -no-rodata-only libcplusplus.s >$@

libcplusplus-no-sfi-pad.fis:	libcplusplus.s rewrite.pl x86_common.pm sizes.pm
	perl rewrite.pl -no-sand libcplusplus.s >$@

libcplusplus-no-sfi-noop.fis:	libcplusplus.s rewrite.pl x86_common.pm sizes.pm
	perl rewrite.pl -nop-only libcplusplus.s >$@

libcplusplus-no-sfi-noop-jo.fis:	libcplusplus.s rewrite.pl x86_common.pm sizes.pm
	perl rewrite.pl -nop-only -jump-only libcplusplus.s >$@

libcplusplus-no-sfi-pushf.fis:	libcplusplus.s rewrite.pl x86_common.pm sizes.pm
	perl rewrite.pl -pushf-and-nop libcplusplus.s >$@

libcplusplus-no-sfi-pushf-jo.fis:	libcplusplus.s rewrite.pl x86_common.pm sizes.pm
	perl rewrite.pl -pushf-and-nop -jump-only libcplusplus.s >$@

%-nstr.s: %.s rewrite-stringops.pl
	perl rewrite-stringops.pl $*.s >$@

%.fis:	%-nstr.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl $*-nstr.s >$@

%-noop.fis:	%-nstr.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -nop-only $*-nstr.s >$@

%-pushf.fis:	%-nstr.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -pushf-and-nop $*-nstr.s >$@

%-jo.fis:	%-nstr.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl -jump-only $*-nstr.s >$@

%.mo:	%.fis
	$(AS) $*.fis -o $@

crtbegin.o: crtbegin.S
	$(AS) crtbegin.S -o crtbegin.o

crtend.o: crtend.S
	$(AS) crtend.S -o crtend.o

%-cpp-mod.fio:	%-cpp-mod.mo libc.mo crtbegin.o crtend.o libcplusplus.mo link-c++.x
	ld $(SECTION) -T link-c++.x crtbegin.o libc.mo libcplusplus.mo $*-cpp-mod.mo crtend.o -o $*-cpp-mod.fio

%-noop.fio:	%-noop.mo libc-noop.mo
	ld $(SECTION) libc-noop.mo $*-noop.mo -o $@

%-pushf.fio:	%-pushf.mo libc-pushf.mo
	ld $(SECTION) libc-pushf.mo $*-pushf.mo -o $@

%-jo.fio:	%-jo.mo libc-jo.mo
	ld $(SECTION) libc-jo.mo $*-jo.mo -o $@

%.fio:	%.mo libc.mo
	ld $(SECTION) libc.mo $*.mo -o $@

%.check: %.fio verify.pl x86_common.pm sizes.pm
	objdump -d $*.fio | perl verify.pl 2>&1 | tee $*.check

%-no-stubs-ebx.s:	%-no-stubs.c
	$(CC) $(OPT) -S $*-no-stubs.c -o $*-no-stubs-ebx.s

%-no-stubs-ebx.s:	%-no-stubs.cc
	$(CXX) $(OPT) -S $*-no-stubs.cc -o $*-no-stubs-ebx.s

%-no-ebx.s:	%-no-stubs.c 
	$(CC) $(OPT) -S $(NO_EBX) $*-no-stubs.c -o $*-no-ebx.s

%-no-ebx.s:	%-no-stubs.cc 
	$(CXX) $(OPT) -S $(NO_EBX) $*-no-stubs.cc -o $*-no-ebx.s

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

%.out:	%.fio %-noop.fio %-pushf.fio %-jo.fio %-raw %-noebx %-pad %-pad-noebx loader
	-/usr/bin/time -f '%e %U %S'    -o $@ ./$*-raw 
	-/usr/bin/time -f '%e %U %S' -a -o $@ ./$*-noebx 
	-/usr/bin/time -f '%e %U %S' -a -o $@ ./$*-pad 
	-/usr/bin/time -f '%e %U %S' -a -o $@ ./$*-pad-noebx 
	-/usr/bin/time -f '%e %U %S' -a -o $@ ./loader $*-noop.fio
	-/usr/bin/time -f '%e %U %S' -a -o $@ ./loader $*-jo.fio
	-/usr/bin/time -f '%e %U %S' -a -o $@ ./loader $*-pushf.fio
	-/usr/bin/time -f '%e %U %S' -a -o $@ ./loader $*.fio

dist:
	rm -rf pittsfield-$(VERSION)
	mkdir pittsfield-$(VERSION)
	cp -p `awk '{print $$1}' MANIFEST` pittsfield-$(VERSION)
	tar cvzf pittsfield-$(VERSION).tar.gz pittsfield-$(VERSION)
