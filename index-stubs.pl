#!/usr/bin/perl

open(STUBS, ">stub-list");
open(WRAPPERS, ">wrappers.h");

sub handle {
    my($name) = @_;
    print STUBS "$name\n";
    print WRAPPERS "(int (*)())&wrap_$name,\n";
}

# GCC internal functions
handle("__udivdi3");
handle("__divdi3");
handle("__umoddi3");
handle("__moddi3");

while (<>) {
    next if m[^//] or /^\s*$/ or /^#/;
    die unless / \*?(\w+)\(/;
    my $name = $1;
    handle($name);
}
