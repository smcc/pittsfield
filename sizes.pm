#!/usr/bin/perl

package sizes;

require Exporter;
our @ISA = "Exporter";
our @EXPORT = qw($code_start $data_start $code_size $code_size
		 $code_end $data_end
		 $data_mask $jump_mask $chunk_size);

# Classic
# my $log_code_size = 24;
# my $code_tag = 0x10;
# my $log_data_size = 24;
# my $data_tag = 0x20;

# Biggest data safe with standard code location
# my $log_code_size = 24;
# my $code_tag = 0x10;
# my $log_data_size = 27;
# my $data_tag = 0x4;

# Biggest data compatible with usual libc location
my $log_code_size = 24;
my $code_tag = 0x10;
my $log_data_size = 29;
my $data_tag = 0x1;

# 1GB data, assumes loader moved to 0xa0000000 or so
my $log_code_size = 24;
my $code_tag = 0x90;
my $log_data_size = 30;
my $data_tag = 0x1;

my $log_chunk_size = 4;


our $code_start = $code_tag << $log_code_size;
our $data_start = $data_tag << $log_data_size;
our $code_size = 1 << $log_code_size;
our $data_size = 1 << $log_data_size;
our $code_end = $code_start + $code_size;
our $data_end = $data_start + $data_size;

our $data_mask = $data_start | ($data_size - 1);
our $jump_mask = ($code_start | ($code_size - 1))
  & ~((1 << $log_chunk_size) - 1);

our $chunk_size = 1 << $log_chunk_size;

#printf "data mask 0x%08x\n", $data_mask;
#printf "jump mask 0x%08x\n", $jump_mask;

sub write_header {
    print "/* Automatically generated from sizes.pl; don't edit directly */\n";
    print "\n";

    printf "#define CODE_START 0x%08x\n", $code_start;
    printf "#define CODE_SIZE  0x%08x\n", $code_size;
    printf "#define DATA_START 0x%08x\n", $data_start;
    printf "#define DATA_SIZE  0x%08x\n", $data_size;
    if ($code_start < $data_start) {
	print "#define CODE_IS_LOWER\n";
    } else {
	print "#define DATA_IS_LOWER\n";
    }
}

