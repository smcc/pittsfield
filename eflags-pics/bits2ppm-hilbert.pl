#!/usr/bin/perl

use strict;

sub min { $_[0] < $_[1] ? $_[0] : $_[1] }
sub max { $_[0] > $_[1] ? $_[0] : $_[1] }

my @img;

my $color;
if ($ARGV[0] eq "r") {
    $color = "\xff\0\0";
} elsif ($ARGV[0] eq "g") {
    $color = "\0\xff\0";
} elsif ($ARGV[0] eq "b") {
    $color = "\0\0\xff";
} else {
    die "Color must be one of r, g, or b";
}

my @bits;
my $char;
open(IN, "$ARGV[1]");
while (defined($char = getc(IN))) {
    if ($char) {
	push @bits, 1;
    } else {
	push @bits, 0;
    }
}
close IN;

my $size = 1;
while ($size * $size < @bits) {
    $size += $size;
}

for my $x (0 .. $size - 1) {
    for my $y (0 .. $size - 1) {
	$img[$x][$y] = "\xff\xff\xff";
    }
}

my($width, $height) = (0, 0);

sub draw {
    my($x1, $y1, $x2, $y2, $i, $hilbert) = @_;
    if ($x1 == $x2 and $y1 == $y2) {
	return if $i >= @bits;
	$width = max($width, $x1 + 1);
	$height = max($height, $y1 + 1);
        $img[$x1][$y1] = $bits[$i] ? $color : "\0\0\0";
    } else {
        my $xm = int(($x1 + $x2) / 2);
        my $ym = int(($y1 + $y2) / 2);
	my $skip = ($xm - $x1 + 1) * ($ym - $y1 + 1);
	my($i0, $i1, $i2, $i3) = ($i, $i + $skip, $i + 2*$skip, $i + 3*$skip);
        if ($hilbert == 0) {
            draw($x1,   $y1,   $xm, $ym, $i0, 1); # 0
            draw($xm+1, $y1,   $x2, $ym, $i1, 0); # 1
            draw($xm+1, $ym+1, $x2, $y2, $i2, 0); # 3
            draw($x1,   $ym+1, $xm, $y2, $i3, 2); # 2
        } elsif ($hilbert == 1) {
            draw($x1,   $y1,   $xm, $ym, $i0, 0); # 0
            draw($x1,   $ym+1, $xm, $y2, $i1, 1); # 2
            draw($xm+1, $ym+1, $x2, $y2, $i2, 1); # 3
            draw($xm+1, $y1,   $x2, $ym, $i3, 3); # 1
        } elsif ($hilbert == 2) {
            draw($xm+1, $ym+1, $x2, $y2, $i0, 3); # 3
            draw($xm+1, $y1,   $x2, $ym, $i1, 2); # 1
            draw($x1,   $y1,   $xm, $ym, $i2, 2); # 0
            draw($x1,   $ym+1, $xm, $y2, $i3, 0); # 2
        } elsif ($hilbert == 3) {
            draw($xm+1, $ym+1, $x2, $y2, $i0, 2); # 3
            draw($x1,   $ym+1, $xm, $y2, $i1, 3); # 2
            draw($x1,   $y1,   $xm, $ym, $i2, 3); # 0
            draw($xm+1, $y1,   $x2, $ym, $i3, 1); # 1
        }
    }
}

draw(0, 0, $size-1, $size-1, 0, 0);

print "P6 $width $height 255\n";

for my $y (0 .. $height - 1) {
    for my $x (0 .. $width - 1) {
	die unless length($img[$x][$y]) == 3;
	print $img[$x][$y];
    }
}
