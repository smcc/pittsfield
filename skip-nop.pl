while (<>) {
    my $raw = substr($_, 10, 22);
    next if $raw =~ /^8d b4 26 00 00 00 00 \t/;
    next if $raw =~ /^8d b6 00 00 00 00  /;
    next if $raw =~ /^8d 74 26 00  /;
    next if $raw =~ /^8d 76 00  /;
    next if $raw =~ /^89 f6  /;
    next if $raw =~ /^90  /;
    print;
}
