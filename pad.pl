#!/usr/bin/perl

# **** This file is obsolete: use the -padonly option to rewrite.pl instead

use strict;

use x86_common;

sub insn_len {
    my($line) = @_;
    if ($line =~ /^\t(inc|dec|pop|push)l\t$ereg$/o) {
	return 1;
    } elsif ($line =~ /\t(leave|ret|cltd|nop|sahf|cwtl)$/o) {
	return 1;
    } elsif ($line =~ /\t(fld1)$/o) {
	return 2;
    } elsif ($line =~ /\tf(abs|chs|compp|coms|cos|ldz|ld1|sin|sqrt)$/o) {
	return 3;
    } elsif ($line =~ /\tmovl\t$ereg, $ereg$/o) {
	return 2;
    } elsif ($line =~ /\tmov[bl]\t($breg|$ereg), \($ereg\)$/o) {
	return 3;
    } elsif ($line =~ /\t($shift)l\t(%cl, )?$ereg$/) {
	return 2;
    } elsif ($line =~ /\t(i?(mul|div)|neg|not)l\t$ereg$/) {
	return 3;
    } elsif ($line =~ /\t($arith8|mov|test)l\t$ereg, $ereg$/o) {
	return 2;
    } elsif ($line =~ /\t($arith8|mov|test)l\t\($ereg_no_esp\), $ereg$/o) {
	return 2;
    } elsif ($line =~ /\t($arith8|mov|test)l\t\(%esp\), $ereg$/o) {
	return 3;
    } elsif ($line =~ /\t($arith8|test)b\t($imb_sign|$breg), $breg$/o) {
	return 3;
    } elsif ($line =~ /\tf(stp)\t$freg$/o) {
	return 2;
    } elsif ($line =~ /\t($arith8|mov|test)b\t$imb_sign, \($ereg\)$/o) {
	return 3;
    } elsif ($line =~ /\tf\w+\t$freg$/) {
	return 3;
    } elsif ($line =~ /\tf\w+\t\($ereg\)$/) {
	return 3;
    } elsif ($line =~ /\tf\w+\t$freg, $freg$/) {
	return 3;
    } elsif ($line =~ /\tmovl\t\($ereg,$ereg(,[1248])?\), $ereg$/) {
	return 3;
    } elsif ($line =~ /\tmov[sz]bl\t\($ereg\), $ereg$/) {
	return 3;
    } elsif ($line =~ /\tmov[sz]bl\t$breg, ?$ereg$/) {
	return 3;
    } elsif ($line =~ /\t(imul)l\t$ereg, ?$ereg$/o) {
	return 3;
    } elsif ($line =~ /\tfnstsw\t%ax$/) {
	return 3;
    } elsif ($line =~ /\tset$cond\t$breg$/o) {
	return 3;
    } elsif ($line =~ /\t($arith8)[bl]\t$imb_sign, $eb_off$/o) {
	return 4;
    } elsif ($line =~ /\t($shift)l\t$imb_sign, $eb_off$/o) {
	return 4;
    } elsif ($line =~ /\t(mov)b\t$imb_sign, $eb_off$/o) {
	return 4;
    } elsif ($line =~ /\t($arith8|mov)l\t$ereg, $eb_off$/o) {
        return 4;
    } elsif ($line =~ /\t($arith8|mov)l\t$ereg, $b_sign\(%esp\)$/o) {
        return 4;
    } elsif ($line =~ /\t($arith8|$shift)l\t$imb_sign, $ereg$/o) {
        return 3;
    } elsif ($line =~ /\t($arith8|mov|lea)l\t$eb_off, $ereg$/o) {
        return 3;
    } elsif ($line =~ /\t($arith8|mov|lea)l\t$b_sign\(%esp\), $ereg$/o) {
        return 4;
    } elsif ($line =~ /\tleal\t\($ereg,$ereg(,[1248])?\), $ereg$/) {
	return 3;
    } elsif ($line =~ /^\t(inc|dec|push)l\t$eb_off$/o) {
        return 3;
    } elsif ($line =~ /\tf\w+\t$b_sign\($ereg\)$/) {
	return 4;
    } elsif ($line =~ /\tf\w+\t\($ereg,$ereg(,[1248])?\)$/) {
	return 4;
    } elsif ($line =~ /\tf\w+\t$b_sign\($ereg,$ereg(,[1248])?\)$/) {
	return 4;
    } elsif ($line =~ /\tleal\t$b_sign\($ereg,$ereg(,[1248])?\), ?$ereg$/) {
	return 4;
    } elsif ($line =~ /\tmov([zs][bw])?l\t$b_sign\($ereg,$ereg(,[1248])?\), ?$ereg$/) {
	return 5;
    } elsif ($line =~ /\tmov[zs][bw]l\t\($ereg,$ereg(,[1248])?\), ?$ereg$/) {
	return 5;
    } elsif ($line =~ /\t(mov([zs][bw])?|lea)l\t$eb_off, $ereg$/) {
	return 4;
    } elsif ($line =~ /\tjmp\t$label$/) {
	return 5;
    } elsif ($line =~ /\t($arith8|mov)l\t$immed, $ereg$/o) {
	return 6;
    } elsif ($line =~ /\tmovl\t$any_const, %eax$/o) {
	return 5;
    } elsif ($line =~ /\tmovl\t%eax, $any_const$/o) {
	return 5;
    } elsif ($line =~ /\tcall\t\w+$/) {
	return 5;
    } elsif ($line =~ /\tj$cond\t$label$/) {
	return 6;
    } elsif ($line =~ /\tmovl\t$ereg, $e_off$/o) {
	return 6;
    } elsif ($line =~ /\tmovl\t$e_off, $ereg$/o) {
	return 6;
    } elsif ($line =~ /\tmovl\t$any_const, $ereg$/o) {
	return 6;
    } elsif ($line =~ /\tmovl\t$imb_sign, $eb_off$/o) {
	return 7;
    } elsif ($line =~ /\t($arith8)l\t$imb_sign, $e_off$/o) {
	return 7;
    } elsif ($line =~ /\tleal\t$any_const\(,$ereg(,[1248])?\), $ereg$/) {
	return 7;
    } elsif ($line =~ /\tmovl\t$immed, \(%esp\)$/o) {
	return 7;
    } elsif ($line =~ /\tmovl\t$any_const\(($ereg)?,$ereg(,[1248])?\), $ereg$/) {
	return 7;
    } elsif ($line =~ /\t($arith8|mov)l\t$immed, $eb_off$/o) {
	return 7;
    } elsif ($line =~ /\t($arith8|mov)l\t$immed, $b_sign\(%esp\)$/o) {
	return 8;
    } elsif ($line =~ /\tmovl\t$imb_sign, $b_sign\(%esp\)$/o) {
	return 8;
    } elsif ($line =~ /\$(-?\d+|[a-zA-Z_\.]+)/
	     and $line =~ /-?(\d+|[a-zA-Z_\.]+)\(/) {
	return 11;
    } elsif ($line =~ /\t($arith8|test|mov)[wl]\t$immed, $any_const$/o) {
	return 10;
    } else {
	return "8?";
    }
}

my $this_chunk = 0;

sub align {
    print "\t.p2align 4\n";
    $this_chunk = 0;
}

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
	#nop_pad(3); nop_pad(2);
	nop_pad(4); nop_pad(1);
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
	} elsif ($args =~ /^\*($lab_complex|$reg|$any_const)$/) {
	    my $target = $1;
	    #print "\tmovl\t$target, %ebx\n";
	    #$real_call .= "\tcall\t*%ebx\n";
	    $real_call = $line;
	    $call_len = 4; # bogus
	} else {
	    die "Strange call $args";
	}
	align();
	nop_pad(16 - $call_len);
	print "$real_call\n";
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
    #s/\n$/ # <$this_chunk>\n/;
    print;
    #print "# <$this_chunk>\n";
}
