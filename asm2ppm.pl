#!/usr/bin/perl

use strict;

my @colors =
  ("\0\0\0",
   "\xff\0\0",
   "\xff\x80\0",
   "\xff\xff\0",
   "\0\xff\0",
   "\0\xff\xff",
   "\0\0\xff",
   "\xff\0\xff",
   "\x80\x80\x80",
   "\x80\0\0",
   "\x80\x40\0",
   "\x80\x80\0",
   "\0\x80\0",
   "\0\x80\x80",
   "\0\0\x80",
   "\x80\0\x80");

my $width = 1024;
my $height = 430;
my @img;

print "P6 $width $height 255\n";

for my $x (0 .. $width - 1) {
    for my $y (0 .. $height - 1) {
	$img[$x][$y] = "\xff\xff\xff";
    }
}

while (<>) {
    next unless /^([0-9a-f]{8}):\t([0-9a-f ]+)\t/;
    my $addr = hex $1;
    $addr -= 0x1000_0000;
    my $bytes = $2;
    my $nop = 0;
    $bytes =~ s/ *$//;
    $nop = 1 if $bytes eq "8d b4 26 00 00 00 00";
    $nop = 1 if $bytes eq "8d bc 27 00 00 00 00";
    $nop = 1 if $bytes eq "8d b6 00 00 00 00";
    $nop = 1 if $bytes eq "8d bf 00 00 00 00";
    $nop = 1 if $bytes eq "8d 74 26 00";
    $nop = 1 if $bytes eq "8d 76 00";
    $nop = 1 if $bytes eq "89 f6";
    $nop = 1 if $bytes eq "90";
    next if $nop;
    $bytes =~ tr/ //d;
    my $len = length($bytes) / 2;
    #printf "%x %x %x\n", $addr, $addr % $width, $addr / $width;
    for my $i ($addr .. $addr + $len - 1) {
	$img[$i % $width][$i / $width] = $colors[$len];
    }
}

for my $y (0 .. $height - 1) {
    for my $x (0 .. $width - 1) {
	die unless length($img[$x][$y]) == 3;
	print $img[$x][$y];
    }
}
