while (<STDIN>) {
    my $raw = substr($_, 10, 22);
    my $nop = 0;
    $nop = 1 if $raw =~ /^8d b4 26 00 00 00 00 \t/;
    $nop = 1 if $raw =~ /^8d bc 27 00 00 00 00 \t/;
    $nop = 1 if $raw =~ /^8d b6 00 00 00 00  /;
    $nop = 1 if $raw =~ /^8d bf 00 00 00 00  /;
    $nop = 1 if $raw =~ /^8d 74 26 00  /;
    $nop = 1 if $raw =~ /^8d 76 00  /;
    $nop = 1 if $raw =~ /^89 f6  /;
    $nop = 1 if $raw =~ /^90  /;
    print if $nop xor ($ARGV[0] ne "-v");
}
