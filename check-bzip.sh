#!/bin/zsh
rm -f /tmp/input.bz2

ulimit -t 5

as $1 -o foo.o
ld --section-start .text=0x10000000 --section-start .data=0x20000000 -e main foo.o -o foo.fio
./loader foo.fio |& fgrep '73237 out'
