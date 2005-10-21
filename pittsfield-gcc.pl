#!/usr/bin/perl

use strict;

# Pretend to be GCC/G++, but run PittSFIeld too.

my $temp_dir = "/tmp";
my $pittsfield_dir = "/afs/csail.mit.edu/u/s/smcc/sfi/pittsfield/pittsfield";
my $rewrite = "$pittsfield_dir/rewrite.pl";
my $perl = "/usr/bin/perl";
my $as = "/usr/bin/as";

sub verbose_command {
    my($prog, @args) = @_;
    my $msg = join(" ", $prog, @args);
    $msg =~ s/$pittsfield_dir/~sfi/g;
    print STDERR "$msg\n";
    system($prog, @args);
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
    die "Only pad-only mode is currently supported" unless $pad_only;
    my($c_file) = @c_files;
    if (!defined $out_file) {
	$out_file = $c_file;
	$out_file =~ s/\.cc?/.o/;
    }
    my $temp_file = "$temp_dir/sfigcc$$";
    verbose_command($real_compiler,
		    "-S", "-o", "$temp_file.s", @args, $c_file);
    verbose_redir_command($perl, "-I$pittsfield_dir", $rewrite,
			  "-padonly", "$temp_file.s",
			  ">$temp_file.fis");
    verbose_command($as, "$temp_file.fis", "-o", $out_file);
} elsif (!$minus_c and !@c_files) {
    # Link objects to executable
    $out_file = "a.out" unless defined $out_file;
    if ($pad_only) {
	verbose_command($real_compiler, "-o", $out_file, @args);
    } else {
	die "Only pad-only mode is currently supported" unless $pad_only;
    }
} else {
    die "That compiler mode is not supported!";
}
