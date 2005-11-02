#!/usr/bin/perl

use strict;

# Pretend to be GCC/G++, but run PittSFIeld too.

my $temp_dir = "/tmp";
my $pittsfield_dir = "/afs/csail.mit.edu/u/s/smcc/sfi/pittsfield/pittsfield";
my $rewrite = "$pittsfield_dir/rewrite.pl";
my $rm_strops = "$pittsfield_dir/rewrite-stringops.pl";
my $verify = "$pittsfield_dir/verify.pl";
my $perl = "/usr/bin/perl";
my $as = "/usr/bin/as";
my $libc_mo = "$pittsfield_dir/libc.mo";
my $ld = "/usr/bin/ld";
my @ld_args = ("--section-start" => ".text=0x90000000",
	       "--section-start" => ".data=0x40000000",
	       "-e" => "main");
my $fake_libc_inc = "$pittsfield_dir/fake-libc-inc";
my $objdump = "/usr/bin/objdump";
my $fio_dir = "/scratch/smcc/pittsfield-fios";
my $loader_c = "$pittsfield_dir/loader.c";
my $highlink_x = "$pittsfield_dir/high-link.x";
my @loader_flags = ("-static", "-lelf", "-lm",
		    "-Wl,-T" => "-Wl,$highlink_x");

sub check_result {
    my($prog, $result) = @_;
    if ($result == -1) {
	die "$prog failed to run: $!";
    } elsif ($result & 127) {
	die "$prog killed by signal " . ($result & 127);
    } elsif ($result) {
	die "$prog failed (return value " . ($result >> 8) . ")";
    }
}

sub verbose_command {
    my($prog, @args) = @_;
    my $msg = join(" ", $prog, @args);
    $msg =~ s/$pittsfield_dir/~sfi/g;
    print STDERR "$msg\n";
    system($prog, @args);
    check_result($prog, $?);
}

# XXX rewrite to fork, open, and exec, avoiding the shell
sub verbose_redir_command {
    my($prog, @args) = @_;
    my $redir = pop @args;
    die unless substr($redir, 0, 1) eq ">";
    my $msg = join(" ", $prog, @args, $redir);
    $msg =~ s/$pittsfield_dir/~sfi/g;
    print STDERR "$msg\n";
    system(join(" ", $prog, @args, $redir));
    check_result($prog, $?);
}

my @args = @ARGV;

my $real_compiler;
if ($0 =~ /gcc/) {
    $real_compiler = "/usr/bin/gcc";
} elsif ($0 =~ /g\+\+/) {
    $real_compiler = "/usr/bin/g++";
} else {
    die "Can't figure out which real compiler to call";
}

my $minus_c = (grep($_ eq "-c", @args) > 0);
@args = grep($_ ne "-c", @args);

my @c_files = grep(/\.cc?$/, @args);
@args = grep(!/\.cc?$/, @args);

my $minus_o_loc = undef;
for (my $i = 0; $i < @args; $i++) {
    $minus_o_loc = $i if $args[$i] eq "-o";
}

my $out_file = undef;
if (defined($minus_o_loc)) {
    $out_file = $args[$minus_o_loc + 1];
    splice(@args, $minus_o_loc, 2);
}

my $pad_only = (grep($_ eq "--pad-only", @args) > 0);
@args = grep($_ ne "--pad-only", @args);

if ($minus_c and @c_files) {
    # Compile source to object
    die "Can only compile one .c file at once" unless @c_files == 1;
    my($c_file) = @c_files;
    if (!defined $out_file) {
	$out_file = $c_file;
	$out_file =~ s/\.cc?/.o/;
    }
    my $basename = $c_file;
    $basename =~ s[^.*/][];
    $basename =~ s/\..*$//;
    my $temp_file = "$temp_dir/sfigcc-$basename$$";
    if (!$pad_only) {
	push @args, "-nostdinc", "-I$fake_libc_inc", "--fixed-ebx";
    }
    verbose_command($real_compiler,
		    "-S", "-o", "$temp_file.s", @args, $c_file);
    verbose_redir_command($perl, "-I$pittsfield_dir", $rm_strops,
			  "$temp_file.s", ">$temp_file-nostr.s");
    verbose_redir_command($perl, "-I$pittsfield_dir", $rewrite,
			  ($pad_only ? "-padonly" : ()),
			  "$temp_file-nostr.s", ">$temp_file.fis");
    verbose_command($as, "$temp_file.fis", "-o", $out_file);
    unlink("$temp_file.s");
    unlink("$temp_file-nostr.s");
    unlink("$temp_file.fis");
} elsif (!$minus_c and !@c_files) {
    # Link objects to executable
    $out_file = "a.out" unless defined $out_file;

    # Remove compiler-ish options
    @args = grep(!/^-O[0123456]$/, @args);
    @args = grep(!/^-f/, @args);

    if ($pad_only) {
	verbose_command($real_compiler, "-o", $out_file, @args);
    } else {
	# Don't try linking any libraries, it won't work
	@args = grep(!/^-l/, @args);

	my $temp_file = "$temp_dir/sfigcc-$out_file$$";
	my $dis_file = "$temp_file.dis";

	my $fio_file = "$fio_dir/$out_file$$.fio";

	verbose_command($ld, "-o", $fio_file, @ld_args, $libc_mo, @args);
	verbose_redir_command($objdump, "-dr", $fio_file, ">$dis_file");
	open(CHECKS, "-|", $perl, "-I$pittsfield_dir", $verify, $dis_file);
	my $okay = 0;
	while (<CHECKS>) {
	    if (/^Checks finished before /) {
		$okay = $_;
	    }
	}
	close CHECKS;
	die "Verification failed" unless $okay;
	print $okay;
	unlink($dis_file);
	verbose_command($real_compiler, "-o", $out_file, "-g",
			qq/-DLOADER_FIO="$fio_file"/, $loader_c,
			@loader_flags);
    }
} else {
    die "That compiler mode is not supported!";
}
