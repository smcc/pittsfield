SECTION:=--section-start .text=0x10000000 --section-start .data=0x20000000 -e main

OPT:=-O3

loader:	loader.c
	gcc -Wall -g loader.c -lelf -o loader

%.fis:	%.c libc.c rewrite.pl x86_common.pm stub-list
	gcc -Wall -S -g -gstabs $(OPT) --fixed-ebx $*.c
	perl rewrite.pl $*.s >$*.fis

%.fio:	%.fis verify.pl x86_common.pm
	as $*.fis -o $*.o
	ld $(SECTION) $*.o -o $*.fio
	objdump -d $*.fio | perl verify.pl

%.out:	%.fio pad.pl
	gcc $(OPT) $*.c -o $*-raw
	gcc $(OPT) --fixed-ebx $*.c -o $*-noebx
	gcc $(OPT) -g -gstabs -S $*.c -o $*-1.s
	perl pad.pl $*-1.s >$*-2.s
	gcc $*-2.s -o $*-pad
	gcc $(OPT) -g -gstabs -S $*.c --fixed-ebx -o $*-1.s
	perl pad.pl $*-1.s >$*-2.s
	gcc $*-2.s -o $*-pad-noebx
	-/usr/bin/time -f '%e %U %S' -o $*.out ./$*-raw 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-noebx 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-pad 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./$*-pad-noebx 
	-/usr/bin/time -f '%e %U %S' -a -o $*.out ./loader $*.fio