SECTION:=--section-start .text=0x90000000 --section-start .data=0x40000000 -e main

OPT:=-O3 -ffast-math
DEBUG:=-g
#CC:=/usr/local/pkg/gcc/gcc-3.2.1/bin/gcc
#export CCACHE_DIR=/scratch2/smcc-extras/ccache
#export CCACHE_LOGFILE=/scratch2/smcc-extras/ccache/ccache.log.smcc
CC:=gcc
#TFF:=/afs/csail.mit.edu/u/s/smcc/old-bin/topformflat
TFF:=./topformflat

loader:	loader.c wrapper.h sizes.h high-link.x
	$(CC) -Wall -g -static loader.c -lelf -lm -Wl,-T -Wl,high-link.x -o loader

wrapper.h: gen-stubs
stub-list: gen-stubs
gen-stubs: stubs.h
	perl index-stubs.pl stubs.h
	touch $@

sizes.h: sizes.pm
	perl -Msizes -e 'sizes::write_header' >sizes.h

%.s:	%-fewer-lines.c libc.c stub-list sizes.h
#       $(CC) -Wall -S $(DEBUG) $(OPT) --fixed-ebx $*.c
	$(CC) -Wall -S $(DEBUG) $(OPT) --fixed-ebx $*-fewer-lines.c -o $*.s

%-fewer-lines.c:	%.c
	$(CC) -E $*.c | $(TFF) 0 | perl -ne 'print unless /^# / or /^\s*$$/' >$*-fewer-lines.c

%-no-stubs.c:	%.c
	$(CC) -DNO_STUBS -E $*.c | $(TFF) 0 | perl -ne 'print unless /^# / or /^\s*$$/' >$*-no-stubs.c

%.fis:	%.s rewrite.pl x86_common.pm sizes.pm stub-list
	perl rewrite.pl $*.s >$*.fis

%.fio:	%.fis
	as $*.fis -o $*.o
	ld $(SECTION) $*.o -o $*.fio

%.check: %.fio verify.pl x86_common.pm sizes.pm
	objdump -d $*.fio | perl verify.pl 2>&1 | tee $*.check

%-no-stubs-ebx.s:	%-no-stubs.c
	$(CC) $(OPT) -S $*-no-stubs.c -o $*-no-stubs-ebx.s

%-no-ebx.s:	%-no-stubs.c
	$(CC) $(OPT) -S --fixed-ebx $*-no-stubs.c -o $*-no-ebx.s

%-raw:	%-no-stubs-ebx.s outside.c
	$(CC) $(OPT) $*-no-stubs-ebx.s outside.c -o $*-raw

%-noebx:	%-no-ebx.s outside.c
	$(CC) $(OPT) $*-no-ebx.s outside.c -o $*-noebx

%-pad.s:	%-no-stubs-ebx.s pad.pl
	perl pad.pl $*-no-stubs-ebx.s >$*-pad.s

%-pad:	%-pad.s outside.c
	$(CC) $*-pad.s outside.c -o $*-pad

%-pad-noebx.s:	%-no-ebx.s pad.pl
	perl pad.pl $*-no-ebx.s >$*-pad-noebx.s

%-pad-noebx:	%-pad-noebx.s outside.c pad.pl
	$(CC) $*-pad-noebx.s outside.c -o $*-pad-noebx

%.out:	%.fio %-raw %-noebx %-pad %-pad-noebx
	-/usr/bin/time -f '%e %U %S' -o $*.out ./$*-raw 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-noebx 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-pad 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-pad-noebx 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./loader $*.fio
