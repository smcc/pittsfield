#!/usr/bin/perl

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
    } elsif ($line =~ /\$-?\d+/ and $line =~ /-?\d+\(/) {
	return 10;
    } else {
	return 8;
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
	s/\n$/ # <$len>\n/;
    }
    print;
}
