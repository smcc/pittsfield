#!/usr/bin/perl

use strict;

use x86_common;

use sizes;

my $do_sandbox = 1;
my $do_no_rodata = 1;
my $do_align = 1;

my $DATA_MASK = sprintf '$0x%08x', $data_mask;
my $JUMP_MASK = sprintf '$0x%08x', $jump_mask;

my $DATA_ANTI_MASK = sprintf '$0x%08x', ~$data_mask;
my $JUMP_ANTI_MASK = sprintf '$0x%08x', ~$jump_mask;

my($DATA_START, $CODE_START);
if ($is_kernel) {
    $DATA_START = '$data_sandbox_start';
    $CODE_START = '$code_sandbox_start';
} else {
    $DATA_START = sprintf '$0x%08x', $data_start;
    $CODE_START = sprintf '$0x%08x', $code_start;
}

my $DO_AND = $do_sandbox && ($style eq "and" || $style eq "andor");
my $DO_OR = $do_sandbox && ($style eq "andor");
my $DO_TEST = $do_sandbox && ($style eq "test");

my $is_main = 0;
if (grep($_ eq "-main", @ARGV)) {
    $is_main = 1;
    @ARGV = grep($_ ne "-main", @ARGV);
}

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

sub emit {
    my($insn, $len) = @_;
    $this_chunk += $len;
    print "\t$insn\n";
    #print "\t$insn # e $this_chunk $len\n";
    if ($this_chunk > $chunk_size) {
	warn "Possible chunk overflow";
	print "\t# XXX\n";
    }
}

sub maybe_align_for {
    my($len) = @_;
    #print "\t# MAF $this_chunk + $len\n";
    align() if $this_chunk + $len > $chunk_size;
}

sub a_emit {
    my($insn, $len) = @_;
    maybe_align_for($len);
    emit($insn, $len);
}

my $label_count = 0;
my $TEST_LEN = 9;

sub emit_test {
    my($reg, $anti_mask) = @_;
    emit("testl\t$anti_mask, $reg", 6 + ($reg eq "(%esp)"));
    emit("jz .LL$label_count", 2);
    emit("int3", 1);
    print ".LL$label_count:\n";
    $label_count++;
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
	a_emit("pushf", 1) if $precious_eflags;
	my $size = "l";
	$size = $1 if $op =~ /([bwl])$/;
	a_emit("leal\t$to, %ebx", 7);
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST + 
			$precious_eflags + 6);
	emit("andl\t$DATA_MASK, %ebx", 6) if $DO_AND;
	emit("orl\t$DATA_START, %ebx", 6) if $DO_OR;
	emit_test("%ebx", $DATA_ANTI_MASK) if $DO_TEST;
	emit("popf", 1) if $precious_eflags;
	emit("$op\t$from, (%ebx)", 6);
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
	a_emit("pushf", 1) if $precious_eflags;
	a_emit("leal\t$target, %ebx", 6);
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST +
			$precious_eflags + 3);
	emit("andl\t$DATA_MASK, %ebx", 6) if $DO_AND;
	emit("orl\t$DATA_START, %ebx", 6) if $DO_OR;
	emit_test("%ebx", $DATA_ANTI_MASK) if $DO_TEST;
	emit("popf", 1) if $precious_eflags;
	emit("$op\t(%ebx)", 3);
	return 1;
    } elsif ($args =~ /^($immed|$reg), ($lword)$/) {
	warn "Skipping bogus direct write $op $args";
	return 1;
    } elsif ($args =~ /^($lword), ($reg)$/) {
	warn "Skipping bogus direct read $op $args";
	return 1;
    } elsif ($do_sandbox and $op eq "ret") {
	$dirty_esp = 0;
	maybe_align_for(7*$DO_AND + 7*$DO_OR + (1 + $TEST_LEN)*$DO_TEST
			+ (length($args) ? 3 : 1));
	emit("andl\t$JUMP_MASK, (%esp)", 7) if $DO_AND;
	emit("orl\t$CODE_START, (%esp)", 7) if $DO_OR;
	emit_test("(%esp)", $JUMP_ANTI_MASK) if $DO_TEST;
	emit("ret $args", (length($args) ? 3 : 1));
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
	    a_emit("movl\t$target, %ebx", 6);
	    $real_call .= "\tandl\t$JUMP_MASK, %ebx\n"
	      if $DO_AND;
	    $real_call .= "\torl\t$CODE_START, %ebx\n"
	      if $DO_OR;
	    if ($DO_TEST) {
		$real_call .= "\ttestl\t$JUMP_ANTI_MASK, %ebx\n";
		$real_call .= "\tjz .LL$label_count\n";
		$real_call .= "\tint3\n";
		$real_call .= ".LL$label_count:\n";
		$label_count++;
	    }
	    $real_call .= "\tcall\t*%ebx\n";
	    $call_len = 2 + (6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
	} else {
	    die "Strange call $args";
	}
	if ($dirty_esp) {
	    my $sb_size = (6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
	    if ($call_len + $sb_size <= $chunk_size) {
		# It will all fit in one chunk
		align();
		nop_pad($chunk_size - $sb_size - $call_len);
		emit("andl\t$DATA_MASK, %esp", 6) if $DO_AND;
		emit("orl\t$DATA_START, %esp", 6) if $DO_OR;
		emit_test("%esp", $DATA_ANTI_MASK) if $DO_TEST;
	    } else {
		# Need two separate chunks
		maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
		emit("andl\t$DATA_MASK, %esp", 6) if $DO_AND;
		emit("orl\t$DATA_START, %esp", 6) if $DO_OR;
		emit_test("%esp", $DATA_ANTI_MASK) if $DO_TEST;
		align();
		nop_pad($chunk_size - $call_len);
	    }
	} else {
	    align();
	    nop_pad($chunk_size - $call_len);
	}
	print "$real_call\n";
	$this_chunk = 0;
	$dirty_esp = 0;
	return 1;
    } elsif ($do_sandbox and $op eq "jmp") {
	if ($args =~ /^\*($lab_complex)$/) {
	    my $target = $1;
	    a_emit("movl\t$target, %ebx", 7);
	} elsif ($args =~ /^\*($reg)$/) {
	    my $target = $1;
	    a_emit("\tmovl\t$target, %ebx", 7);
	} else {
	    return 0;
	}
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST + 2);
	emit("andl\t$JUMP_MASK, %ebx", 6) if $DO_AND;
	emit("orl\t$CODE_START, %ebx", 6) if $DO_OR;
	emit_test("%ebx", $JUMP_ANTI_MASK) if $DO_TEST;
	emit("jmp\t*%ebx", 2);
	return 1;	
    } else {
	return 0;
    }
}

sub print_stubs {
    open(STUBS, "<stub-list");
    my $i = 0;
    while (<STUBS>) {
	my $f = $_;
	chomp $f;
	print ".globl $f\n";
	print "$f:\n";
	printf "\tjmp\t0x%08x\n", $code_start + ($i << $log_chunk_size);
	print "\tpopl\t%ebx\n";
	print "\t.p2align $log_chunk_size\n" if $style eq "test";
	$this_chunk = 0;
	print "\tandl\t$JUMP_MASK, %ebx\n" if $DO_AND;
	print "\torl\t$CODE_START, %ebx\n" if $DO_OR;
	emit_test("%ebx", $JUMP_ANTI_MASK) if $DO_TEST;
	print "\tjmp\t*%ebx\n";
	print "\t.p2align $log_chunk_size\n";
	$this_chunk = 0;
	$i++;
    }
    close STUBS;
}

if ($is_kernel) {
    print scalar <>;
    print qq/\t.section .sandboxed_text, "ax"\n/;
    print "\t.p2align 12\n";
    print ".globl code_sandbox_start\n";
    print "code_sandbox_start:\n";
    print "\tjmp return_from_sandbox\n";
    print "\t.p2align $log_chunk_size\n";
} else {
    while (<>) {
	last if /^\t?\.text/;
	print;
    }
    print;
    if ($is_main) {
	nop_pad($chunk_size - 5);
	print "\tcall main\n"; # 5 bytes
	print "\tret\n";
	print "\t.p2align $log_chunk_size\n";
    }
}

print_stubs if $is_main;

if ($is_kernel) {
    open(EXPORTS, "<export-list");
    while (<EXPORTS>) {
	my $f = $_;
	chomp $f;
	print "stub_$f:\n";
	nop_pad($chunk_size - 5);
	print "\tcall $f\n"; # 5 bytes
	print "\tpopl %ecx\n";
	print "\tjmp return_from_sandbox\n";
	print "\t.p2align $log_chunk_size\n";
    }
    close EXPORTS;
}

my($seen_data) = 0;

while (<>) {
    my $comment;
    if ($is_kernel and /\t\.data/ || /\.section \.rodata/) {
	print qq/\t.section .sandboxed_data, "aw"\n/;
        if (!$seen_data) {
            print "\t.p2align 12\n";
            print ".globl data_sandbox_start\n";
            print "data_sandbox_start:\n";
            $seen_data = 1;
        }
	next;
    } elsif ($is_kernel and /^\t\.text/) {
	print qq/\t.section .sandboxed_text, "ax"\n/;
	next;
    }
    if ($do_no_rodata and /^\s+\.section\s+.rodata/) {
	print ".data\n";
	next;
    } elsif ($do_no_rodata and /^\s+\.section\s+\.note\.GNU-stack/) {
	next;
    }
    if ($dirty_esp
	and /^\t(jmp|cmp|inc|dec|add|and|or|test|shr|s[ah]l|sahf)/) {
	if ($do_sandbox) {
	    maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
	    emit("andl\t$DATA_MASK, %esp", 6) if $DO_AND;
	    emit("orl\t$DATA_START, %esp", 6) if $DO_OR;
	    emit_test("%esp", $DATA_ANTI_MASK) if $DO_TEST;
	}
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
	$comment = "# " . ($dirty_esp?"d":"") . ($precious_eflags?"f":"");
    } elsif (/^\t\.p2align 2/) {
	$this_chunk += 4;
    } elsif (/^($label|\w+):$/) {
	align();
    }
    chomp;
    print "$_ $comment\n";
    if ($do_sandbox and /\t(leave|popl\s+%ebp)$/) {
	a_emit("pushf", 1) if $precious_eflags;
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
	emit("andl\t$DATA_MASK, %ebp", 6) if $DO_AND;
	emit("orl\t$DATA_START, %ebp", 6) if $DO_OR;
	emit_test("%ebp", $DATA_ANTI_MASK) if $DO_TEST;
	a_emit("popf", 1) if $precious_eflags;
    } elsif ($do_sandbox and /^\t(add|sub|lea)l\s+($ereg|$complex), %esp$/) {
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
	emit("andl\t$DATA_MASK, %esp", 6) if $DO_AND;
	emit("orl\t$DATA_START, %esp", 6) if $DO_OR;
	emit_test("%esp", $DATA_ANTI_MASK) if $DO_TEST;
    }
    if (/^\t(test|cmp|dec)/) {
	$precious_eflags = 1;
    } elsif (/^\tj$cond/) {
	$precious_eflags = 0;
    }
    print "# <$.>\n" if ($. % 10) == 0;
}
