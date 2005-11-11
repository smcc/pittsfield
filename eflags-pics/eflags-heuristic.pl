#!/usr/bin/perl

use strict;

use x86_common;

my $safer = 0;
if (grep($_ eq "-safer", @ARGV)) {
    $safer = 1;
    @ARGV = grep($_ ne "-safer", @ARGV);
}

my $live = 0;

print $live;

while (<>) {
    if (/^\t(test|cmp|dec|and)/) {
	$live = 1;
    } elsif (/^\tj($cond)\t/ and !($safer && $1 eq "e")) {
	$live = 0;
    }
    print $live;
}
