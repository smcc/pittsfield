#!/bin/zsh
#ulimit -t 5

as $1 -o foo.o
ld --section-start .text=0x90000000 --section-start .data=0x40000000 -e main foo.o -o foo.fio
./loader foo.fio |& head | fgrep 'refresh_errno'
