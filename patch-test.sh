dir=/g6/users/smcc/class/cs255/proj/spike2
cp $dir/factor-mod.s foo.s
patch -f foo.s -o foo.fis <$1
as foo.fis -o foo.o
ld --section-start .text=0x10000000 --section-start .data=0x20000000 -e main foo.o -o foo.fio
$dir/loader foo.fio | fgrep "81064801949712479: 81064801949712479"
