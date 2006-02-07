#!/usr/bin/perl

package sizes;

use strict;

require Exporter;
our @ISA = "Exporter";
our @EXPORT = qw($code_start $data_start $code_size $data_size
		 $code_end $data_end
		 $style $is_kernel
		 $data_mask $jump_mask $log_chunk_size $chunk_size
		 $size_macro compute_sizes @allowed_sizes);

# Log sandbox size  Size
# 24                16MB
# 25                32MB
# 26                64MB
# 27                128MB
# 28                256MB
# 29                512MB
# 30                1GB

#                 code       data
my %sizes = #    size tag  size tag
  ("classic" => [24, 0x10, 24, 0x20], # Classic (code and data 16MB each)
   "medium"  => [24, 0x10, 27, 0x4],  # Biggest data (128MB) safe with
                                      # standard code location
   "large"   => [24, 0x90, 30, 0x1],  # 1GB data, assumes loader moved
  );                                  # to 0xa0000000 or so

our @allowed_sizes = keys %sizes;

our $is_kernel = 0;      # 1 may not work everywhere
our $style = "and";      # "andor" and "test" may not work everywhere
our $log_chunk_size = 4; # 5 may not work everywhere

our($code_start, $data_start, $code_size, $data_size,
    $code_end, $data_end,
    $stub_size, $log_stub_size,
    $data_mask, $jump_mask, $log_chunk_size, $chunk_size);

our $size_macro;

sub compute_sizes {
    my($size) = @_;
    my($log_code_size, $code_tag, $log_data_size, $data_tag)
      = @{$sizes{$size}};

    $size_macro = "SFI_" . uc($size);

    $code_start = $code_tag << $log_code_size;
    $data_start = $data_tag << $log_data_size;
    $code_size = 1 << $log_code_size;
    $data_size = 1 << $log_data_size;
    $code_end = $code_start + $code_size;
    $data_end = $data_start + $data_size;

    $data_mask = $data_start | ($data_size - 1);
    $jump_mask = ($code_start | ($code_size - 1))
      & ~((1 << $log_chunk_size) - 1);

    $chunk_size = 1 << $log_chunk_size;

    $log_stub_size = $log_chunk_size;
    $log_stub_size = $log_chunk_size+1 if $style eq "test";

    $stub_size = 1 << $log_stub_size;
}

sub write_header {
    print "/* Automatically generated from sizes.pm; don't edit directly */\n";
    print "\n";

    for my $size (keys %sizes) {
	compute_sizes($size);
	print "#ifdef $size_macro\n";
	printf "#define CODE_START 0x%08xU\n", $code_start unless $is_kernel;
	printf "#define CODE_SIZE  0x%08xU\n", $code_size;
	printf "#define DATA_START 0x%08xU\n", $data_start unless $is_kernel;
	printf "#define DATA_SIZE  0x%08xU\n", $data_size;
	printf "#define DATA_MASK  0x%08xU\n", $data_mask;
	printf "#define JUMP_MASK  0x%08xU\n", $jump_mask;
	printf "#define CHUNK_SIZE  0x%x\n", $chunk_size;
	printf "#define STUB_SIZE  0x%x\n", $stub_size;
	if (!$is_kernel) {
	    if ($code_start < $data_start) {
		print "#define CODE_IS_LOWER\n";
	    } else {
		print "#define DATA_IS_LOWER\n";
	    }
	}
	print "#endif /* $size_macro */\n\n";
    }
}

sub ld_flags {
    return ("--section-start", sprintf(".text=0x%08x", $code_start),
	    "--section-start", sprintf(".data=0x%08x", $data_start));
}

sub write_ld_flags {
    my($size) = @_;
    compute_sizes($size);
    return join(" ", ld_flags());
}

1;
