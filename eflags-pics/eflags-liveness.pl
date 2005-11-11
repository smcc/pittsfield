#!/usr/bin/perl

use strict;

use x86_common;

my @bits;

my $live = 0;

push @bits, $live;

open(IN, "-|", "/usr/bin/tac", $ARGV[0]);

while (<IN>) {
    if (/^\t(j|set)($cond)\t/) {
	$live = 1;
    } elsif (/^\t(cmp|test|inc|dec|add|sub|and|or|xor|test|s[ah]r|s[ah]l|sahf)/) {
	$live = 0;
    } elsif (/^\t(jmp|call|ret)/) {
	$live = 0;
    }
    push @bits, $live;
}

print reverse @bits;
