#!/usr/bin/perl

use strict;

use x86_common;

sub insn_len {
    my($line) = @_;
    if ($line =~ /^\t(inc|dec|pop|push)l\t$ereg$/o) {
	return 1;
    } elsif ($line =~ /\t(leave|ret)$/o) {
	return 1;
    } elsif ($line =~ /\tmovl\t$ereg, $ereg$/o) {
	return 2;
    } elsif ($line =~ /\t($arith8|mov)l\t$ereg, $ereg$/o) {
	return 2;
    } elsif ($line =~ /\t($arith8|mov)l\t$ereg, $eb_off$/o) {
        return 3;
    } elsif ($line =~ /\t($arith8|$shift)l\t$imb_sign, $ereg$/o) {
        return 3;
    } elsif ($line =~ /\t($arith8|mov|lea)l\t$eb_off, $ereg$/o) {
        return 3;
    } elsif ($line =~ /^\t(inc|dec|push)l\t$eb_off$/o) {
        return 3;
    } elsif ($line =~ /\tj($cond|mp)\t$label$/) {
	return 6;
    } else {
	return 10;
    }
}

my $this_chunk = 0;

sub align {
    print "\t.p2align 4\n";
    $this_chunk = 0;
}

my $dirty_esp = 0;

sub nop_pad {
    my($bytes) = @_;
    if ($bytes == 1) {
	print "\tnop\n";
    } elsif ($bytes == 2) {
	print "\tmov\t%esi, %esi\n";
    } elsif ($bytes == 3) {
	#print "\tlea\t(%esi), %esi\n";
	print "\t.byte 0x8d, 0x76, 0\n";
    } elsif ($bytes == 4) {
	#print "\tlea\t0(%esi,1), %esi\n";
	print "\t.byte 0x8d, 0x74, 0x26, 0\n";
    } elsif ($bytes == 5) {
	nop_pad(3); nop_pad(2);
    } elsif ($bytes == 6) {
	print "\t.byte 0x8d, 0xb6, 0, 0, 0, 0\n";
    } elsif ($bytes == 7) {
	print "\t.byte 0x8d, 0xb4, 0x26, 0, 0, 0, 0\n";
    } elsif ($bytes == 8) {
	nop_pad(4); nop_pad(4);
    } elsif ($bytes > 8) {
	while ($bytes > 7) {
	    nop_pad(7);
	    $bytes -= 7;
	}
	nop_pad($bytes);
    }
}

sub maybe_rewrite {
    my($line) = @_;
    chomp $line;
    return 0 unless $line =~ /^\t([a-z]+)(\t(.*))?/;
    my($op, $args) = ($1, $3);
    if ($op eq "call") {
	my $real_call;
	my $call_len;
	if ($args =~ /^\.?\w+$/) {
	    $real_call = $line;
	    $call_len = 5;
	} elsif ($args =~ /^\*($complex)$/) {
	    my $target = $1;
	    print "\tmovl\t$target, %ebx\n";
	    $real_call = "\tcall\t*%ebx\n";
	    $call_len = 2; # andl; FF 13
	} elsif ($args =~ /^\*($reg)$/) {
	    my $target = $1;
	    print "\tmovl\t$target, %ebx\n";
	    $real_call = "\tcall\t*%ebx\n";
	    $call_len = 2; # andl; FF 13
	}
	align();
	nop_pad(16 - $call_len);
	print "$real_call\n";
	$dirty_esp = 0;
	return 1;
    } else {
	return 0;
    }
}

while (<>) {
    last if /^\t?\.text/;
    print;
}
print;

while (<>) {
    if (/^\t[a-z]/) {
	my $len = insn_len($_);
	if ($this_chunk + $len > 16) {
	    align();
	}
	next if maybe_rewrite($_);
	$this_chunk += $len;
    } elsif (/^\t\.p2align 2/) {
	$this_chunk += 4;
    } elsif (/^($label|\w+):$/) {
	align();
    }
    print;
    #print "# <$this_chunk>\n";
}
