#!/usr/bin/perl

use strict;

# Pretend to be GCC/G++, but run PittSFIeld too.

use FindBin;
use lib "$FindBin::RealBin";
my $pittsfield_dir = $FindBin::RealBin;
use sizes;

my $temp_dir = "/tmp";
my $rewrite = "$pittsfield_dir/rewrite.pl";
my $rm_strops = "$pittsfield_dir/rewrite-stringops.pl";
my $verify = "$pittsfield_dir/verify.pl";
my $perl = "/usr/bin/perl";
my $binutils_dir = $ENV{"PITTSFIELD_BINUTILS_DIR"} || "/usr/bin";
my $as = "$binutils_dir/as";
my $libc_c = "$pittsfield_dir/libc.c";
my $libcplusplus_cc = "$pittsfield_dir/libcplusplus.cc";
my $ld = "$binutils_dir/ld";
my $fake_libc_inc = "$pittsfield_dir/sandbox-include";
my $objdump = "$binutils_dir/objdump";
my $fio_dir = "$temp_dir/pittsfield-fios";
my $loader_c = "$pittsfield_dir/loader.c";
my $highlink_x = "$pittsfield_dir/high-link.x";
my $linkcpp_x = "$pittsfield_dir/link-c++.x";
my $crtbegin_S = "$pittsfield_dir/crtbegin.S";
my $crtend_S = "$pittsfield_dir/crtend.S";
my @loader_flags = ("-static", "-lelf", "-lm");
my @c_opt = ("-O3");
my @cxx_opt = ("-O3", "-fno-exceptions", "-fno-rtti");
my $gcc = "gcc";
my $gxx = "g++";

my $vx32_dir = "<not_supported>";
my $vx32_libc_inc = "$vx32_dir/cinc";
my $vx32_libc_a = "$vx32_dir/clib/libc-pittsfield.a";
my $vx32_crt0 = "$vx32_dir/clib/pittsfield/crt0.o";

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
    $msg =~ s/$vx32_dir/~vx32/g;
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
    $msg =~ s/$vx32_dir/~vx32/g;
    print STDERR "$msg\n";
    system(join(" ", $prog, @args, $redir));
    check_result($prog, $?);
}

my @args = @ARGV;

my @size_flags = ();
for my $size (@allowed_sizes) {
    if (grep($_ eq "--size-$size", @args)) {
	@size_flags = ("-size-$size");
	compute_sizes($size);
	@args = grep($_ ne "--size-$size", @args);
	if ($size eq "large") {
	    push @loader_flags, "-Wl,-T" => "-Wl,$highlink_x";
	}
    }
}

my @ld_args = sizes::ld_flags();

for my $arg (@args) {
    if ($arg =~ /^--gcc=(.*)/) {
	$gcc = $1;
    } elsif ($arg =~ /^--g\+\+=(.*)/) {
	$gxx = $1;
    }
}
@args = grep(!/^--g(cc|\+\+)=/, @args);

my $cplusplus_mode = 0;
my $real_compiler = $gcc;
if ($0 =~ /\+\+/) {
    $cplusplus_mode = 1;
    $real_compiler = $gxx;
}

my $minus_c = (grep($_ eq "-c", @args) > 0);
@args = grep($_ ne "-c", @args);

my @c_files = grep(/\.cc?$/, @args);
@args = grep(!/\.cc?$/, @args);

my $minus_o_loc = undef;
for (my $i = 0; $i < @args; $i++) {
    $minus_o_loc = $i if $args[$i] eq "-o";
}

my $fio_only = 0;

my $out_file = undef;
if (defined($minus_o_loc)) {
    $out_file = $args[$minus_o_loc + 1];
    splice(@args, $minus_o_loc, 2);
}

if ($out_file =~ /\.fio$/) {
    $fio_only = 1;
}

my $pad_only = (grep($_ eq "--pad-only", @args) > 0);
@args = grep($_ ne "--pad-only", @args);

my $no_sfi = "";
for my $kind ("base", "noschd", "noebx", "pad", "noop", "pushf") {
    if (grep($_ eq "--no-sfi=$kind", @args)) {
	@args = grep($_ ne "--no-sfi=$kind", @args);
	$no_sfi = $kind;
    }
}

my $jump_only = (grep($_ eq "--jump-only", @args) > 0);
@args = grep($_ ne "--jump-only", @args);

my $vx32 = (grep($_ eq "--vx32", @args) > 0);
@args = grep($_ ne "--vx32", @args);

my $crt0 = (grep($_ eq "--crt0", @args) > 0);
@args = grep($_ ne "--crt0", @args);

my $threadsafe_return = (grep($_ eq "--threadsafe-return", @args) > 0);
@args = grep($_ ne "--threadsafe-return", @args);

my($libc_mo_name, $libcplusplus_mo_name) = ("libc.mo", "libcplusplus.mo");
if ($no_sfi or $jump_only) {
    my $ext = "";
    $ext .= "-no-sfi-$no_sfi" if $no_sfi;
    $ext .= "-jo" if $jump_only;
    $libc_mo_name = "libc$ext.mo";
    $libcplusplus_mo_name = "libcplusplus$ext.mo";
}

sub assemble_file {
    my($s_file, $o_file) = @_;
    verbose_command($as, $s_file, "-o", $o_file);
}

sub compile_file {
    my($c_file, $out_file, $other_args, $cxx_mode) = @_;
    my @args = @$other_args;
    my $basename = $c_file;
    $basename =~ s[^.*/][];
    $basename =~ s/\..*$//;
    my $temp_file = "$temp_dir/sfigcc-$basename$$";
    if (!$pad_only) {
	push @args, "-nostdinc";
	if ($vx32) {
	    push @args, "-I$vx32_libc_inc";
	} else {
	    push @args, "-I$fake_libc_inc";
	}
	push @args, "-fno-schedule-insns2" unless $no_sfi eq "base";
	push @args, "--fixed-ebx"
	  unless $no_sfi eq "base" or $no_sfi eq "noschd";
    }
    my @rewrite_flags = (@size_flags);
    if ($pad_only) {
	@rewrite_flags = ("-padonly");
    } elsif ($no_sfi eq "base" or $no_sfi eq "noschd" or $no_sfi eq "noebx") {
	@rewrite_flags = ("-no-rodata-only");
    } elsif ($no_sfi eq "pad") {
	@rewrite_flags = ("-no-sand");
    } elsif ($no_sfi eq "noop") {
	@rewrite_flags = ("-nop-only");
    } elsif ($no_sfi eq "pushf") {
	@rewrite_flags = ("-pushf-and-nop");
    }
    if ($jump_only) {
	push @rewrite_flags, "-jump-only";
    }
    if ($crt0) {
	push @rewrite_flags, ($vx32 ? "-vx32main" : "-main");
    }
    if ($threadsafe_return) {
	push @rewrite_flags, "-threadsafe-return";
    }
    verbose_command(($cxx_mode ? $gxx : $gcc),
		    "-S", "-o", "$temp_file.s", @args, $c_file);
    verbose_redir_command($perl, "-I$pittsfield_dir", $rm_strops,
			  "$temp_file.s", ">$temp_file-nostr.s");
    verbose_redir_command($perl, "-I$pittsfield_dir", $rewrite,
			  @rewrite_flags, "$temp_file-nostr.s",
			  ">$temp_file.fis");
    assemble_file("$temp_file.fis", $out_file);
    unlink("$temp_file.s");
    unlink("$temp_file-nostr.s");
    unlink("$temp_file.fis");
}

if ($minus_c and @c_files) {
    # Compile source to object
    die "Can only compile one .c file at once" unless @c_files == 1;
    my($c_file) = @c_files;
    if (!defined $out_file) {
	$out_file = $c_file;
	$out_file =~ s/\.cc?/.o/;
    }
    compile_file($c_file, $out_file, [@args], $cplusplus_mode);
} elsif (!$minus_c and !@c_files) {
    # Link objects to executable
    if (not defined $out_file) {
	if ($fio_only) {
	    $out_file = "a.out.fio";
	} else {
	    $out_file = "a.out";
	}
    }

    # Remove compiler-ish options
    @args = grep(!/^-O[0123456]$/, @args);
    @args = grep(!/^-f/, @args);

    if ($pad_only) {
	verbose_command($real_compiler, "-o", $out_file, @args);
    } else {
	# Don't try linking any libraries, it won't work
	@args = grep(!/^-l/, @args);

	my $basename = $out_file;
	$basename =~ s[^.*/][];
	$basename =~ s/\..*$//;

	my $temp_file = "$temp_dir/sfigcc-$basename$$";
	my $dis_file = "$temp_file.dis";
	
	my $fio_file;
	if ($fio_only) {
	    $fio_file = $out_file;
	} else {
	    $fio_file = "$fio_dir/$out_file$$.fio";
	}

	my @to_unlink;

	my $libc_mo = "$temp_file-$libc_mo_name";
	if (!$vx32) {
	    $crt0 = 1;
	    compile_file($libc_c, $libc_mo, \@c_opt, 0);
	    push @to_unlink, $libc_mo;
	    $crt0 = 0;
	}

	if ($cplusplus_mode) {
	    # Besides linking in our equivalent of libstdc++.a, we
	    # also need to do more linker trickery to make calling
	    # static constructors work.
	    my $libcplusplus_mo = "$temp_file-$libcplusplus_mo_name";
	    compile_file($libcplusplus_cc, $libcplusplus_mo, \@cxx_opt, 1);
	    push @to_unlink, $libcplusplus_mo;

	    my $crtbegin_o = "$temp_file-crtbegin.o";
	    assemble_file($crtbegin_S, $crtbegin_o);
	    push @to_unlink, $crtbegin_o;
	    my $crtend_o = "$temp_file-crtend.o";
	    assemble_file($crtend_S, $crtend_o);
	    push @to_unlink, $crtend_o;

	    unshift @ld_args, $crtbegin_o;
	    push @ld_args, $crtend_o;
	    push @ld_args, $libcplusplus_mo;
	    push @ld_args, ("-T" => "$linkcpp_x");
	}

	my @start_objs;
	if ($vx32) {
	    @start_objs = ($vx32_crt0);
	    push @args, $vx32_libc_a;
	} else {
	    @start_objs = ($libc_mo);
	}

	verbose_command($ld, "-o", $fio_file, @start_objs, @ld_args, @args);
	if (!$no_sfi and !$jump_only) {
	    verbose_redir_command($objdump, "-dr", $fio_file, ">$dis_file");
	    open(CHECKS, "-|", $perl, "-I$pittsfield_dir", $verify,
		 @size_flags, $dis_file);
	    my $okay = 0;
	    while (<CHECKS>) {
		if (/^Checks finished before /) {
		    $okay = $_;
		} else {
		    die "Verification failed: $_";
		}
	    }
	    close CHECKS;
	    die "Verification failed" unless $okay;
	    print $okay;
	    unlink($dis_file);
	}
	if (!$fio_only) {
	    verbose_command($gcc, "-o", $out_file, "-g",
			    "-D$size_macro",
			    qq/-DLOADER_FIO="$fio_file"/, $loader_c,
			    @loader_flags);
	}
	for my $temp_f (@to_unlink) {
	    unlink($temp_f);
	}
    }
} else {
    die "That compiler mode is not supported!";
}
