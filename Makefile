VERSION:=0.7

OPT:=-O2 -ffast-math
CXXFLAGS := -fno-exceptions -fno-rtti
DEBUG:=-g
AS:=as
NO_EBX := --fixed-ebx
NO_SCHD := -fno-schedule-insns2

# Adjust for your preferred version of GCC, at least 3.3 suggested:
# CC  := gcc-3.3
# CXX := g++-3.3
CC  := gcc-4.8 -m32
CXX := g++-4.8 -m32

# Adjust to location of libdisasm, if you want the C verifier
# VERIFY_CFLAGS := -I../libdisasm-0.23/libdisasm -DVERIFY
# VERIFY_LDFLAGS := ../libdisasm-0.23/libdisasm/.libs/libdisasm.a

# Adjust to location of 32-bit libelf, if non-standard
# VERIFY_CFLAGS := -I$(HOME)/soft/i386/lib/libelf/0.8.13/include/libelf
# VERIFY_LDFLAGS := -L$(HOME)/soft/i386/lib/libelf/0.8.13/lib -Wl,-rpath $(HOME)/soft/i386/lib/libelf/0.8.13/lib

# These flags work if you have the 64-bit version of libelf installed
# system-wide, and copy a 32-bit libelf.a into the current directory.
VERIFY_CFLAGS := 
VERIFY_LDFLAGS := -L.

#VERIFY_CFLAGS :=
#VERIFY_LDFLAGS :=

SIZE ?= classic
REWRITE := perl rewrite.pl -size-$(SIZE)
VERIFY_PL := perl verify.pl -size-$(SIZE)
SIZE_DEFINE := $(shell perl -Msizes -e 'compute_sizes("$(SIZE)"); print "-D$$size_macro"')

SFI_CC  := perl pittsfield-gcc.pl --gcc=$(CC) --size-$(SIZE)
SFI_CXX := perl pittsfield-g++.pl --gcc=$(CC) --g++=$(CXX) --size-$(SIZE)

CFLAGS := $(DEBUG) $(OPT) $(SIZE_DEFINE)

# .PRECIOUS: %.o %.s %.fis %.fio %-raw %-noebx %-pad %-pad-noebx

all: loader-large loader-medium loader-classic hw.fio hw-c++.fio

loader-large:	loader.c wrappers.h sizes.h high-link.x
	$(CC) -O2 -Wall $(VERIFY_CFLAGS) -DSFI_LARGE -g -static loader.c $(VERIFY_LDFLAGS) -lelf -lm -Wl,-T -Wl,high-link.x -o $@

loader-medium:	loader.c wrappers.h sizes.h
	$(CC) -O2 -Wall $(VERIFY_CFLAGS) -DSFI_MEDIUM -g loader.c $(VERIFY_LDFLAGS) -lelf -lm -o $@

loader-classic:	loader.c wrappers.h sizes.h high-link.x
	$(CC) -O2 -Wall $(VERIFY_CFLAGS) -DSFI_CLASSIC -g loader.c $(VERIFY_LDFLAGS) -lelf -lm -o $@

clean:
	rm -f *.o *.s *.fis *.fio *.mo *-raw *-noebx *-pad *.check *-mod.out
	rm -f loader gen-stubs sizes.h stub-list vx32-stub-list wrappers.h
	rm -f loader-large loader-medium loader-classic
	rm -rf sandbox-include
	rm -f pittsfield-g++.pl

wrappers.h: gen-stubs
stub-list: gen-stubs
gen-stubs: stubs.h
	perl index-stubs.pl stubs.h
	touch $@

sizes.h: sizes.pm
	perl -Msizes -e 'sizes::write_header' >sizes.h

sandbox-include/.setup: libc-h-copies.txt stubs.h libc.h stubs.h float.h fstream.h iostream.h
	mkdir -p sandbox-include
	mkdir -p sandbox-include/sys
	for f in `cat libc-h-copies.txt`; do \
            cp libc.h sandbox-include/$$f; done
	for f in stubs.h float.h fstream.h iostream.h; do \
            cp $$f sandbox-include/$$f; done
	cp stat.h sandbox-include/sys/stat.h
	touch $@

pittsfield-g++.pl: pittsfield-gcc.pl
	cp $< $@

%.mo: %.c sandbox-include/.setup stub-list
	$(SFI_CC) $(CFLAGS) -c $< -o $@

%.mo: %.cc pittsfield-g++.pl sandbox-include/.setup stub-list
	$(SFI_CXX) $(CXXFLAGS) $(CFLAGS) -c $< -o $@

%-c++.fio: %-c++.mo pittsfield-g++.pl libc.c libcplusplus.cc
	$(SFI_CXX) $(CXXFLAGS) $< -o $@

%.fio: %.mo libc.c
	$(SFI_CC) $< -o $@

%.check: %.fio verify.pl x86_common.pm sizes.pm
	objdump -d $*.fio | $(VERIFY_PL) 2>&1 | tee $*.check

# begin no dist
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
	perl -ni -e 'print unless /# begin no dist/ .. /# end no dist/ or /# end no dist/' pittsfield-$(VERSION)/Makefile
	tar cvzf pittsfield-$(VERSION).tar.gz pittsfield-$(VERSION)
# end no dist
