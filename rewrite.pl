#!/usr/bin/perl

use strict;

use x86_common;

use sizes;

my $do_sandbox = 1;
my $do_no_rodata = 1;
my $do_align = 1;
my $nop_only = 0;
my $pushf_and_nop = 0;
my $threadsafe_return = 0;

my $is_main = 0;
my $stubs_list;
if (grep($_ eq "-main", @ARGV)) {
    $is_main = 1;
    $stubs_list = "stub-list";
    @ARGV = grep($_ ne "-main", @ARGV);
} elsif (grep($_ eq "-vx32main", @ARGV)) {
    $is_main = 1;
    $stubs_list = "vx32-stub-list";
    @ARGV = grep($_ ne "-vx32main", @ARGV);
}

if (grep($_ eq "-padonly", @ARGV)) {
    $do_align = 1;
    $do_sandbox = $do_no_rodata = 0;
    @ARGV = grep($_ ne "-padonly", @ARGV);
} elsif (grep($_ eq "-no-sand", @ARGV)) {
    $do_align = $do_no_rodata = 1;
    $do_sandbox = 0;
    @ARGV = grep($_ ne "-no-sand", @ARGV);
} elsif (grep($_ eq "-no-rodata-only", @ARGV)) {
    $do_no_rodata = 1;
    $do_sandbox = $do_align = 0;
    @ARGV = grep($_ ne "-no-rodata-only", @ARGV);
} elsif (grep($_ eq "-nop-only", @ARGV)) {
    $nop_only = 1;
    @ARGV = grep($_ ne "-nop-only", @ARGV);
} elsif (grep($_ eq "-pushf-and-nop", @ARGV)) {
    $nop_only = 1;
    $pushf_and_nop = 1;
    @ARGV = grep($_ ne "-pushf-and-nop", @ARGV);
}

if (grep($_ eq "-threadsafe-return", @ARGV)) {
    $threadsafe_return = 1;
    @ARGV = grep($_ ne "-threadsafe-return", @ARGV);
}

for my $size (@allowed_sizes) {
    if (grep($_ eq "-size-$size", @ARGV)) {
	compute_sizes($size);
	@ARGV = grep($_ ne "-size-$size", @ARGV);
    }
}

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

my($jump_sandbox, $data_sandbox);
if (grep($_ eq "-jump-only", @ARGV)) {
    ($jump_sandbox, $data_sandbox) = ($do_sandbox, 0);
    @ARGV = grep($_ ne "-jump-only", @ARGV);
} else {
    ($jump_sandbox, $data_sandbox) = ($do_sandbox, $do_sandbox);
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

# Heuristic approximation to EFLAGS liveness computed forward
my $forward_eflags_live = 0;

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
    my($insn, $len, $can_nop) = @_;
    $this_chunk += $len;
    if ($nop_only and $can_nop and
	!($pushf_and_nop && $insn =~ /^(push|pop)f/)) {
	nop_pad($len);
    } else {
	print "\t$insn\n";
	#print "\t$insn # e $this_chunk $len\n";
    }
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
    my($insn, $len, $can_nop) = @_;
    maybe_align_for($len);
    emit($insn, $len, $can_nop);
}

my $label_count = 0;
my $TEST_LEN = 9;

sub emit_test {
    my($reg, $anti_mask) = @_;
    emit("testl\t$anti_mask, $reg", 6 + ($reg eq "(%esp)"), 1);
    emit("jz .LL$label_count", 2, 1);
    emit("int3", 1, 1);
    print ".LL$label_count:\n";
    $label_count++;
}

sub maybe_rewrite {
    my($line) = @_;
    chomp $line;
    return 0 unless $line =~ /^\t([a-z]+)(\t(.*))?/;
    my($op, $args) = ($1, $3);
    if ($data_sandbox and $args =~ /^($immed|$reg), ($lab_complex)$/ and
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
	a_emit("pushf", 1, 1) if $precious_eflags;
	my $size = "l";
	$size = $1 if $op =~ /([bwl])$/;
	a_emit("leal\t$to, %ebx", 7, 0);
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST + 
			$precious_eflags + 6);
	emit("andl\t$DATA_MASK, %ebx", 6, 1) if $DO_AND;
	emit("orl\t$DATA_START, %ebx", 6, 1) if $DO_OR;
	emit_test("%ebx", $DATA_ANTI_MASK, 1) if $DO_TEST;
	emit("popf", 1, 1) if $precious_eflags and $DO_TEST;
	emit("$op\t$from, (%ebx)", 6, 0);
	emit("popf", 1, 1) if $precious_eflags and !$DO_TEST;
	return 1;
    } elsif ($data_sandbox and $args =~ /^$lab_complex$/ and $op =~
	     /^(inc|dec|i?div|i?mul|$shift|neg|not)[bwl]|set$cond|$fstore|$fcstore/) {
	my $target = $args;
	return 0 if $op =~ /^lea[blw]$/;
	if ($target =~ /^(-?\d+)\((%e[bs]p)\)$/) {
	    my($offset, $base_reg) = ($1, $2);
	    if ($base_reg eq "%ebp" and abs($offset) < 32768) {
		return 0;
	    } elsif ($base_reg eq "%esp" and abs($offset) < 127) {
		return 0;
	    }
	} elsif ($target =~ /^\((%e[sb]p)\)$/) {
	    return 0;
	}
	my $early_popf = ($DO_TEST || $op =~ /^set$cond$/);
	a_emit("pushf", 1, 1) if $precious_eflags;
	a_emit("leal\t$target, %ebx", 6, 0);
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST +
			$precious_eflags + 3);
	emit("andl\t$DATA_MASK, %ebx", 6, 1) if $DO_AND;
	emit("orl\t$DATA_START, %ebx", 6, 1) if $DO_OR;
	emit_test("%ebx", $DATA_ANTI_MASK, 1) if $DO_TEST;
	emit("popf", 1, 1) if $precious_eflags and $early_popf;
	emit("$op\t(%ebx)", 3, 0);
	# XXX When is it the right thing to popf after the OP?
	emit("popf", 1, 1) if $precious_eflags and !$early_popf;
	return 1;
    } elsif ($args =~ /^($immed|$reg), ($lword)$/) {
	warn "Skipping bogus direct write $op $args";
	return 1;
    } elsif ($args =~ /^($lword), ($reg)$/) {
	warn "Skipping bogus direct read $op $args";
	return 1;
    } elsif ($jump_sandbox and $op eq "ret") {
	if (!$threadsafe_return) {
	    $dirty_esp = 0;
	    maybe_align_for(7*$DO_AND + 7*$DO_OR + (1 + $TEST_LEN)*$DO_TEST
			    + (length($args) ? 3 : 1));
	    emit("andl\t$JUMP_MASK, (%esp)", 7, 1) if $DO_AND;
	    emit("orl\t$CODE_START, (%esp)", 7, 1) if $DO_OR;
	    emit_test("(%esp)", $JUMP_ANTI_MASK, 1) if $DO_TEST;
	    emit("ret $args", (length($args) ? 3 : 1), 0);
	} else {
	    # The old, slow way:
	    my $do_extra_pop = length($args) > 0;
	    emit("popl\t%ebx", 1);
	    if ($do_extra_pop) {
		# XXX is this the right length?
		emit("addl\t$args, %esp", 3);
		maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
		emit("andl\t$DATA_MASK, %esp", 6, 1) if $DO_AND;
		emit("orl\t$DATA_START, %esp", 6, 1) if $DO_OR;
		emit_test("%esp", $DATA_ANTI_MASK, 1) if $DO_TEST;
	    }
	    maybe_align_for(7*$DO_AND + 7*$DO_OR + (1 + $TEST_LEN)*$DO_TEST
	    		+ 2);
	    emit("andl\t$JUMP_MASK, %ebx", 7) if $DO_AND;
	    emit("orl\t$CODE_START, %ebx", 7) if $DO_OR;
	    emit_test("%ebx", $JUMP_ANTI_MASK) if $DO_TEST;
	    emit("jmp *%ebx", 2);
	}
	return 1;
    } elsif ($op eq "call") {
	my $real_call = "";
	my $sandbox_len = 0;
	my $call_len;
	return 0 if !$jump_sandbox and !$do_align;
	if ($args =~ /^\.?[\w.]+$/) {
	    $real_call = $line;
	    $call_len = 5;
	} elsif ($args =~ /^\*($lab_complex|$reg|$any_const)$/) {
	    my $target = $1;
	    if (!$jump_sandbox) {
		$real_call = $line;
		if ($args =~ /\*$eb_off$/) {
		    $call_len = 3;
		} else {
		    $call_len = 4; # bogus
		}
	    } else {
		a_emit("movl\t$target, %ebx", 6, 0);
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
		$sandbox_len = 6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST;
		if ($nop_only) {
		    # Will emit equivalent # of nops below
		    $real_call = "";
		}
		$real_call .= "\tcall\t*%ebx\n";
		$call_len = $sandbox_len + 2;
	    }
	} else {
	    die "Strange call $args";
	}
	if ($dirty_esp) {
	    my $sb_size = (6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
	    if ($call_len + $sb_size <= $chunk_size) {
		# It will all fit in one chunk
		align();
		nop_pad($chunk_size - $sb_size - $call_len);
		emit("andl\t$DATA_MASK, %esp", 6, 1) if $DO_AND;
		emit("orl\t$DATA_START, %esp", 6, 1) if $DO_OR;
		emit_test("%esp", $DATA_ANTI_MASK, 1) if $DO_TEST;
	    } else {
		# Need two separate chunks
		maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
		emit("andl\t$DATA_MASK, %esp", 6, 1) if $DO_AND;
		emit("orl\t$DATA_START, %esp", 6, 1) if $DO_OR;
		emit_test("%esp", $DATA_ANTI_MASK, 1) if $DO_TEST;
		align();
		nop_pad($chunk_size - $call_len);
	    }
	} else {
	    align();
	    nop_pad($chunk_size - $call_len);
	}
	if ($nop_only) {
	    nop_pad($sandbox_len);
	}
	print "$real_call\n";
	$this_chunk = 0;
	$dirty_esp = 0;
	return 1;
    } elsif ($jump_sandbox and $op eq "jmp") {
	if ($args =~ /^\*($lab_complex)$/) {
	    my $target = $1;
	    a_emit("movl\t$target, %ebx", 7, 0);
	} elsif ($args =~ /^\*($reg)$/) {
	    my $target = $1;
	    a_emit("\tmovl\t$target, %ebx", 7, 0);
	} else {
	    return 0;
	}
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST + 2);
	emit("andl\t$JUMP_MASK, %ebx", 6, 1) if $DO_AND;
	emit("orl\t$CODE_START, %ebx", 6, 1) if $DO_OR;
	emit_test("%ebx", $JUMP_ANTI_MASK, 1) if $DO_TEST;
	emit("jmp\t*%ebx", 2, 0);
	return 1;	
    } else {
	return 0;
    }
}

sub print_stubs {
    my $script_loc = $0;
    $script_loc =~ s([^/]+$)();
    open(STUBS, "<${script_loc}$stubs_list")
      or die "Can't open ${script_loc}$stubs_list: $!";
    my $i = 0;
    while (<STUBS>) {
	my $f = $_;
	chomp $f;
	print ".globl $f\n";
	print "$f:\n";
	#printf "\tjmp\t0x%08x\n", $code_start + ($i << $log_chunk_size);
	print "\tnop\n" for 1 .. 5;
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

my @eflags_live_before;

sub eflags_liveness {
    my($in_file) = @_;
    my @bits;

    my $live = 0;
    push @bits, $live;

    open(IN, "-|", "/usr/bin/tac", $in_file);
    while (<IN>) {
	if (/^\t(j|set)($cond)\t/) {
	    $live = 1;
	} elsif (/^\t(cmp|test|inc|dec|add|sub|and|or|xor|test|s[ah]r|s[ah]l|sahf)/) {
	    $live = 0;
	} elsif (/^\t(jmp|call|ret)/) {
	    $live = 0;
	}
	push @bits, $live;
    }
    close IN;

    @eflags_live_before = reverse @bits;
}

my $done_early = 1;

my $in_file = $ARGV[0];

eflags_liveness($in_file);

open(INPUT, "<$in_file");

if ($is_kernel) {
    print scalar <INPUT>;
    print qq/\t.section .sandboxed_text, "ax"\n/;
    print "\t.p2align 12\n";
    print ".globl code_sandbox_start\n";
    print "code_sandbox_start:\n";
    print "\tjmp return_from_sandbox\n";
    print "\t.p2align $log_chunk_size\n";
} else {
    while (<INPUT>) {
	if (/^\t?\.text/ or /linkonce.*\"ax/) {
	    # OK, there's real code in this file
	    $done_early = 0;
	    last;
	}
	if ($do_no_rodata and /^\s+\.section\s+.rodata/) {
	    print ".data\n";
	    next;
	} else {
	    print;
	}
    }
    print;
    if ($is_main) {
	nop_pad($chunk_size - 7);
	print "\tpushl %edx\n"; # argv: 1 byte
	print "\tpushl %ecx\n"; # argc: 1 byte
	print "\tcall _start\n"; # 5 bytes
	print "\taddl \$8,%esp\n"; # pop argc and argv
	print "\tpushl %eax\n"; # return of main => arg to exit
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
	emit("andl\t$DATA_MASK, %esp", 6, 1) if $DO_AND;
	emit("orl\t$DATA_START, %esp", 6, 1) if $DO_OR;
	emit_test("%esp", $DATA_ANTI_MASK, 1) if $DO_TEST;
	print "\tcall exit\n"; # will not return
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

exit if $done_early;

my($seen_data) = 0;

while (<INPUT>) {
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
    } elsif ($do_no_rodata and /^\s+\.section\s+.gnu.linkonce.r/) {
	print ".data\n";
	next;
    } elsif ($do_no_rodata and /^\s+\.section\s+\.note\.GNU-stack/) {
	next;
    }
    if ($dirty_esp
	and /^\t(jmp|cmp|inc|dec|add|sub|and|or|xor|test|s[ah]r|s[ah]l|sahf|neg)/) {
	if ($data_sandbox) {
	    maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
	    emit("andl\t$DATA_MASK, %esp", 6, 1) if $DO_AND;
	    emit("orl\t$DATA_START, %esp", 6, 1) if $DO_OR;
	    emit_test("%esp", $DATA_ANTI_MASK, 1) if $DO_TEST;
	}
	$dirty_esp = 0;
    }
    if (/^\t(push|pop)/) {
	$dirty_esp = 0;
    } elsif (/, %esp/) {
	$dirty_esp = 1;
    }
    if (/^\t(test|cmp|dec)/) {
	$forward_eflags_live = 0; # We kill the old ones
    }
    if (/^\trep/) {
	warn "Noticed rep at line $.: string ops not supported";
    }
    $precious_eflags = ($forward_eflags_live || $eflags_live_before[$. - 1]);
    if (/^\t[a-z]/) {
	my $len = insn_len($_);
	if ($this_chunk + $len > $chunk_size) {
	    align();
	}
	next if maybe_rewrite($_);
	$this_chunk += $len;
	$comment = "# " . ($dirty_esp?"d":"") . ($forward_eflags_live?"f":"") .
	  ($eflags_live_before[$.-1]?"F":"");
    } elsif (/^\t\.p2align 2/) {
	$this_chunk += 4;
    } elsif (/^\t\.p2align/) {
	# Ignore other alignment directives introduced by the compiler
	next;
    } elsif (/^($label|[\w.]+):$/) {
	align();
    }
    chomp;
    print "$_ $comment\n";
    #print "$_\n";
    if ($data_sandbox and /\t(leave|popl\s+%ebp)$/
	|| (/, %ebp/ and !/\tmovl\t%esp, %ebp$/)) {
	a_emit("pushf", 1, 1) if $precious_eflags;
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
	emit("andl\t$DATA_MASK, %ebp", 6, 1) if $DO_AND;
	emit("orl\t$DATA_START, %ebp", 6, 1) if $DO_OR;
	emit_test("%ebp", $DATA_ANTI_MASK, 1) if $DO_TEST;
	a_emit("popf", 1, 1) if $precious_eflags;
    } elsif ($data_sandbox and
	     (/^\t(add|sub|lea)l\s+($ereg|$complex), %esp$/ or
	      /^\t(add|sub|lea)l\s+\$(0x[0-9a-f]+|\d+), %esp$/
	       && hex($2) > 255)) {
	maybe_align_for(6*$DO_AND + 6*$DO_OR + $TEST_LEN*$DO_TEST);
	emit("andl\t$DATA_MASK, %esp", 6, 1) if $DO_AND;
	emit("orl\t$DATA_START, %esp", 6, 1) if $DO_OR;
	emit_test("%esp", $DATA_ANTI_MASK, 1) if $DO_TEST;
    }
    if (/^\t(test|cmp)/) {
	$forward_eflags_live = 1;
    } elsif (/^\t(j$cond|set$cond|jmp|call|ret)\t/) {
	$forward_eflags_live = 0;
    } elsif (/^\tsbbl\t%(\w+), %(\w+)/ and $1 eq $2) {
	# cmpl op1, op2; sbbl %out,%out 
	# idiom for "%out = ((op1 >= op2) ? 0 : -1)" used by GCC 4
	$forward_eflags_live = 0;
    }
    #print "# <$.>\n" if ($. % 10) == 0;
}
