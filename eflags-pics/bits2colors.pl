#!/usr/bin/perl

my @colors =
  ("black",   # 000
   "blue",    # 001
   "green",   # 010
   "cyan",    # 011
   "red",     # 100
   "magenta", # 101
   "yellow",  # 110
   "white",   # 111
  );

open(R, "<$ARGV[0]");
open(G, "<$ARGV[1]");
open(B, "<$ARGV[2]");

while (defined($r = getc(R))) {
    $g = getc(G);
    $b = getc(B);
    print "# ", $colors[4*$r + 2*$g + $b], "\n";
}
