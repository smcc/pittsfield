#!/usr/bin/perl

use strict;

use x86_common;

use sizes;

my $do_sandbox = 1;
my $do_no_rodata = 1;
my $do_align = 1;

my $DATA_MASK = sprintf '$0x%08x', $data_mask;
my $JUMP_MASK = sprintf '$0x%08x', $jump_mask;
my $DATA_START = sprintf '$0x%08x', $data_start;
my $CODE_START = sprintf '$0x%08x', $code_start;

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
    print "\t.p2align $log_chunk_size\n" if $do_align;
    #print "\t.p2align $log_chunk_size, 0x90\n" if $do_align;
    $this_chunk = 0;
}

my $dirty_esp = 0;
my $precious_eflags = 0;

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
	#print "\t.byte 0x36, 0x8d, 0x74, 0x26, 0\n";
	nop_pad(4); nop_pad(1);
    } elsif ($bytes == 6) {
	print "\t.byte 0x8d, 0xb6, 0, 0, 0, 0\n";
    } elsif ($bytes == 7) {
	print "\t.byte 0x8d, 0xb4, 0x26, 0, 0, 0, 0\n";
    } elsif ($bytes == 8) {
	#print "\t.byte 0x36, 0x8d, 0xb4, 0x26, 0, 0, 0, 0\n";
	#nop_pad(4); nop_pad(4);
	nop_pad(7); nop_pad(1);
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
    if ($do_sandbox and $args =~ /^($immed|$reg), ($lab_complex)$/ and
	$op !~ /^(test|cmp)[bwl]?$/) {
	my($from, $to) = ($1, $2);
	return 0 if $op =~ /^lea[blw]$/;
	if ($to =~ /^(-?\d*)\((%e[sb]p)\)$/) {
	    my($offset, $base_reg) = ($1, $2);
	    if ($base_reg eq "%ebp" and abs($offset) < 32767) {
		unless ($from =~ /\$(-?\d+)/ and abs($1) > 127) {
		    return 0;
		}
	    } elsif ($base_reg eq "%esp" and abs($offset) < 127) {
		return 0;
	    }
	} elsif ($to =~ /^\((%e[sb]p)\)$/) {
	    return 0;
	}
 	align() if $this_chunk + 7 + 6*($style eq "andor") > $chunk_size;
	print "\tpushf\n" if $precious_eflags;
	my $size = "l";
	$size = $1 if $op =~ /([bwl])$/;
	print "\tleal\t$to, %ebx\n";
	align();
	print "\tandl\t$DATA_MASK, %ebx\n" if $style =~ /and/;
	print "\torl\t$DATA_START, %ebx\n" if $style =~ /or/;
	print "\t$op\t$from, (%ebx)\n";
	$this_chunk = 12 + 6*($style eq "andor");
	$this_chunk++, print "\tpopf\n" if $precious_eflags;
	return 1;
    } elsif ($do_sandbox and $args =~ /^$lab_complex$/ and $op =~
	     /^(inc|dec|i?div|i?mul|$shift|neg|not)[bwl]|set$cond|$fstore/) {
	my $target = $args;
	return 0 if $op =~ /^lea[blw]$/;
	if ($target =~ /^(-?\d+)\((%e[bs]p)\)$/) {
	    my($offset, $base_reg) = ($1, $2);
	    if ($base_reg eq "%ebp" and  abs($offset) < 32768) {
		return 0;
	    } elsif ($base_reg eq "%esp" and abs($offset) < 127) {
		return 0;
	    }
	} elsif ($target =~ /^\((%e[sb]p)\)$/) {
	    return 0;
	}
	if ($precious_eflags) {
	    align() if $this_chunk + 1 > $chunk_size;
	    print "\tpushf\n";
	    $this_chunk++;
	}
	align() if $this_chunk + 6 > $chunk_size;
	print "\tleal\t$target, %ebx\n";
	align();
	print "\tandl\t$DATA_MASK, %ebx\n" if $style =~ /and/;
	print "\torl\t$DATA_START, %ebx\n" if $style =~ /or/;
	print "\t$op\t(%ebx)\n";
	$this_chunk = 9 + 6*($style eq "andor");
	$this_chunk++, print "\tpopf\n" if $precious_eflags;
	return 1;
    } elsif ($args =~ /^($immed|$reg), ($lword)$/) {
	warn "Skipping bogus direct write $op $args";
	return 1;
    } elsif ($args =~ /^($lword), ($reg)$/) {
	warn "Skipping bogus direct read $op $args";
	return 1;
    } elsif ($do_sandbox and $op eq "ret") {
	$dirty_esp = 0;
	align();
	print "\tandl\t$JUMP_MASK, (%esp)\n" if $style =~ /and/;
	print "\torl\t$CODE_START, (%esp)\n" if $style =~ /or/;
	print "\tret $args\n";
	$this_chunk = 9 + 6*($style eq "andor");
	return 1;
    } elsif ($op eq "call") {
	my $real_call = "";
	my $call_len;
	return 0 if !$do_sandbox and !$do_align;
	if ($args =~ /^\.?\w+$/) {
	    $real_call = $line;
	    $call_len = 5;
	} elsif ($args =~ /^\*($lab_complex|$reg|$any_const)$/) {
	    my $target = $1;
	    print "\tmovl\t$target, %ebx\n";
	    $real_call .= "\tandl\t$JUMP_MASK, %ebx\n"
	      if $do_sandbox and $style =~ /and/;
	    $real_call .= "\torl\t$CODE_START, %ebx\n"
	      if $do_sandbox and $style =~ /or/;
	    $real_call .= "\tcall\t*%ebx\n";
	    $call_len = $do_sandbox ? 8 + 6*($style eq "andor") : 2;
	} else {
	    die "Strange call $args";
	}
	align();
	if ($dirty_esp) {
	    nop_pad($chunk_size - (6 + 6*($style eq "andor")) - $call_len);
	    print "\tandl\t$DATA_MASK, %esp\n" if $style =~ /and/;
	    print "\torl\t$DATA_START, %esp\n" if $style =~ /or/;
	} else {
	    nop_pad($chunk_size - $call_len);
	}
	print "$real_call\n";
	$dirty_esp = 0;
	return 1;
    } elsif ($do_sandbox and $op eq "jmp") {
	if ($args =~ /^\*($lab_complex)$/) {
	    my $target = $1;
	    print "\tmovl\t$target, %ebx\n";
	} elsif ($args =~ /^\*($reg)$/) {
	    my $target = $1;
	    print "\tmovl\t$target, %ebx\n";
	} else {
	    return 0;
	}
	align();
	print "\tandl\t$JUMP_MASK, %ebx\n" if $style =~ /and/;
	print "\torl\t$CODE_START, %ebx\n" if $style =~ /or/;
	print "\tjmp\t*%ebx\n";
	$this_chunk = 8 + 6*($style eq "andor");
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
nop_pad($chunk_size - 5);
print "\tcall main\n"; # 5 bytes
print "\tret\n";
print "\t.p2align 5\n";

open(STUBS, "<stub-list");
my $i = 0;
while (<STUBS>) {
    my $f = $_;
    chomp $f;
    print "$f:\n";
    printf "\tjmp\t0x%08x\n", $code_start + ($i << $log_chunk_size);
    print "\tpopl\t%ebx\n";
    print "\tandl\t$JUMP_MASK, %ebx\n" if $style =~ /and/;
    print "\torl\t$CODE_START, %ebx\n" if $style =~ /or/;
    print "\tjmp\t*%ebx\n";
    print "\t.p2align $log_chunk_size\n";
    $i++;
}
close STUBS;

while (<>) {
    my $comment;
    if ($do_no_rodata and /^\s+\.section\s+.rodata/) {
	print ".data\n";
	next;
    } elsif ($do_no_rodata and /^\s+\.section\s+\.note\.GNU-stack/) {
	next;
    }
    if ($do_sandbox and $dirty_esp and /^\t(jmp|cmp|inc|dec|add|and|or|test|shr|s[ah]l|sahf)/) {
 	align() if $this_chunk + 6 + 6*($style eq "andor") > $chunk_size;
	print("\tandl\t$DATA_MASK, %esp\n"), $this_chunk+=6 if $style =~ /and/;
	print("\torl\t$DATA_START, %esp\n"), $this_chunk+=6 if $style =~ /or/;
	$dirty_esp = 0;
    }
    if (/^\t(push|pop)/) {
	$dirty_esp = 0;
    } elsif (/, %esp/) {
	$dirty_esp = 1;
    }
    if (/^\t(test|cmp|dec)/) {
	$precious_eflags = 0; # We kill the old ones
    }
    if (/^\tcld$/) {
	warn "Noticed cld at line $.: string ops not supported";
    }
    if (/^\t[a-z]/) {
	my $len = insn_len($_);
	if ($this_chunk + $len > $chunk_size) {
	    align();
	}
	next if maybe_rewrite($_);
	$this_chunk += $len;
	$comment = "# $len" . ($dirty_esp?"d":"") . ($precious_eflags?"f":"");
    } elsif (/^\t\.p2align 2/) {
	$this_chunk += 4;
    } elsif (/^($label|\w+):$/) {
	align();
    }
    chomp;
    print "$_ $comment\n";
    if ($do_sandbox and /\t(leave|popl\s+%ebp)$/) {
	my $size = ($precious_eflags ? 2 : 0) + 6 + 6*($style eq "andor");
	align() if $this_chunk + $size > $chunk_size;
	print "\tpushf\n" if $precious_eflags;
	print "\tandl\t$DATA_MASK, %ebp\n" if $style =~ /and/;
	print "\torl\t$DATA_START, %ebp\n" if $style =~ /or/;
	print "\tpopf\n" if $precious_eflags;
	$this_chunk += $size;
    } elsif ($do_sandbox and /^\t(add|sub|lea)l\s+($ereg|$complex), %esp$/) {
	align() if $this_chunk + 6 + 6*($style eq "andor") > $chunk_size;
	print "\tandl\t$DATA_MASK, %esp\n" if $style =~ /and/;
	print "\torl\t$DATA_START, %esp\n" if $style =~ /or/;
	$this_chunk += 6 + 6*($style eq "andor");
    }
    if (/^\t(test|cmp|dec)/) {
	$precious_eflags = 1;
    } elsif (/^\tj$cond/) {
	$precious_eflags = 0;
    }
    #print "# <$this_chunk>\n";
}
