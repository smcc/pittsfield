#SECTION:=--section-start .text=0x90000000 --section-start .data=0x40000000 -e main
SECTION:=--section-start .text=0x10000000 --section-start .data=0x20000000 -e main

OPT:=-O3 -ffast-math
DEBUG:=-g
CC:=gcc
CXX:=g++ -fno-exceptions -fno-rtti
AS:=as
TFF:=./topformflat

.PRECIOUS: %.o %.s %.fis %.fio %-raw %-noebx %-pad %-pad-noebx

loader:	loader.c wrappers.h sizes.h high-link.x
	@#$(CC) -Wall -g -static loader.c -lelf -lm -Wl,-T -Wl,high-link.x -o loader
	$(CC) -Wall -g loader.c -lelf -lm -o loader

wrapper.h: gen-stubs
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
	perl rewrite.pl -main libc.s >libc.fis

libc-no-stubs.o:	libc.c libc.h
	$(CC) $(OPT) -DNO_STUBS -c $< -o $@

%.fis:	%.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl $*.s >$*.fis

%.o:	%.fis
	$(AS) $*.fis -o $*.o

%.fio:	%.o libc.o
	ld $(SECTION) libc.o $*.o -o $*.fio

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

%-pad.s:	%-no-stubs-ebx.s pad.pl
	perl pad.pl $*-no-stubs-ebx.s >$*-pad.s

%-pad:	%-pad.s libc-no-stubs.o outside.c
	$(CC) $*-pad.s libc-no-stubs.o outside.c -o $*-pad -lm

%-pad-noebx.s:	%-no-ebx.s pad.pl
	perl pad.pl $*-no-ebx.s >$*-pad-noebx.s

%-pad-noebx:	%-pad-noebx.s libc-no-stubs.o outside.c pad.pl
	$(CC) $*-pad-noebx.s libc-no-stubs.o outside.c -o $*-pad-noebx -lm

%.out:	%.fio %-raw %-noebx %-pad %-pad-noebx loader
	-/usr/bin/time -f '%e %U %S' -o $*.out ./$*-raw 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-noebx 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-pad 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-pad-noebx 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./loader $*.fio
