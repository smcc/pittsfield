#!/usr/bin/perl

open(STUBS, ">stub-list");
open(VX32STUBS, ">vx32-stub-list");
open(WRAPPERS, ">wrappers.h");

my $forbidden = 1;

sub handle {
    my($name) = @_;
    print STUBS "$name\n";
    if ($name =~ /vx32/ or $name eq "fail_check" or $name =~ /^__/) {
	print VX32STUBS "$name\n";
    } else {
	print VX32STUBS "forbidden$forbidden\n";
	$forbidden++;
    }
    print WRAPPERS "(int (*)())&wrap_$name,\n";
}

# GCC internal functions
handle("__udivdi3");
handle("__divdi3");
handle("__umoddi3");
handle("__moddi3");
handle("__floatdidf");
handle("__fixdfdi");

while (<>) {
    next if m[^//] or /^\s*$/ or /^#/;
    die unless / \*?(\w+)\(/;
    my $name = $1;
    handle($name);
}
