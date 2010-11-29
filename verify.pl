use strict;

use x86_common;

use sizes;

for my $size (@allowed_sizes) {
    if (grep($_ eq "-size-$size", @ARGV)) {
	compute_sizes($size);
	@ARGV = grep($_ ne "-size-$size", @ARGV);
    }
}

my $DATA_MASK = sprintf '$0x%08x', $data_mask;
my $JUMP_MASK = sprintf '$0x%08x', $jump_mask;
my $DATA_START = sprintf '$0x%08x', $data_start;
my $CODE_START = sprintf '$0x%08x', $code_start;

sub USE_ESP         () { 1 <<  0 }
sub IJUMP           () { 1 <<  1 }
sub IWRITE          () { 1 <<  2 }
sub IREAD           () { 1 <<  3 }
sub USE_EBP         () { 1 <<  4 }
sub CHANGE_EBP      () { 1 <<  5 }
sub EBX_DATA_SAFE   () { 1 <<  6 }
sub EBX_CODE_SAFE   () { 1 <<  7 }
sub JUMP            () { 1 <<  8 }
sub EBP_DATA_SAFE   () { 1 <<  9 }
sub CHANGE_ESP      () { 1 << 10 }
sub BUMP_ESP        () { 1 << 11 }
sub STACK_TOP_SAFE  () { 1 << 12 }
sub EBX_DATA_XSAFE  () { 1 << 13 }
sub EBX_CODE_XSAFE  () { 1 << 14 }
sub STACK_TOP_XSAFE () { 1 << 15 }
sub EBP_DATA_XSAFE  () { 1 << 16 }
sub ESP_DATA_SAFE   () { 1 << 17 }

my($data_safety, $code_safety, $stack_top_safety, $ebp_safety, $esp_safety);
if ($style eq "and") {
    $data_safety = EBX_DATA_SAFE;
    $code_safety = EBX_CODE_SAFE;
    $stack_top_safety = STACK_TOP_SAFE;
    $ebp_safety = EBP_DATA_SAFE;
    $esp_safety = USE_ESP;
} elsif ($style eq "andor") {
    $data_safety = EBX_DATA_XSAFE;
    $code_safety = EBX_CODE_XSAFE;
    $stack_top_safety = STACK_TOP_XSAFE;
    $ebp_safety = EBP_DATA_XSAFE;
    $esp_safety = ESP_DATA_SAFE;
} else {
    warn "Verifier is incomplete in TEST mode";
    $data_safety = 0;
    $code_safety = 0;
    $stack_top_safety = 0;
    $ebp_safety = 0;
    $esp_safety = 0;
}

sub signed_word {
    my($x) = @_;
    if ($x =~ /^-0x([0-9a-f]+)$/) {
	return -hex($1);
    } elsif ($x =~ /^0x([0-9a-f]+)$/) {
	# interpret unsigned hex as signed
	return unpack("i", pack("I", hex($1)));
    } elsif ($x =~ /^-?(\d+)$/) {
	return $x;
    } elsif ($x eq "") {
	return 0;
    } else {
	use Carp;
	confess "Unsupported signed word format '$x'";
    }
}

sub check_insn {
    my($op, $args, $safety, $unsafety) = @_;
    if ($args eq "") {
	if ($op eq "nop") {
	    return 0;
	} elsif ($op eq "ret") {
	    if ($safety & $stack_top_safety) {
		return JUMP|USE_ESP|CHANGE_ESP;
	    } else {
		return JUMP|IJUMP|USE_ESP|CHANGE_ESP;
	    }
	} elsif ($op eq "leave") {
	    # move %ebp, %esp; pop %ebp
	    if (not ($unsafety & CHANGE_EBP)) {
		return USE_ESP|CHANGE_EBP;
	    } else {
		return USE_ESP|CHANGE_EBP|CHANGE_ESP;
	    }
	} elsif ($op eq "pushf" or $op eq "popf") {
	    return USE_ESP|CHANGE_ESP;
	} elsif ($op =~ /^$convert$/) {
	    return 0;
	} elsif ($op =~ /^($fconst|$funary)$/) {
	    return 0;
	} elsif ($op eq "sahf") {
	    return 0;
	} elsif ($op eq "int3") {
	    return 0;
	} else {
	    die "Unknown none-ary insn $op";
	}
    } elsif ($args =~ /^($reg)$/) {
	my $target = $1;
	my $change = 0;
	$change |= CHANGE_EBP if $target =~ /bp/;
	$change |= CHANGE_ESP if $target =~ /sp/;
	if ($op eq "push") {
	    return USE_ESP|CHANGE_ESP;
	} elsif ($op eq "pop") {
	    return USE_ESP|$change|CHANGE_ESP;
	} elsif ($op eq "inc" or $op eq "dec") {
	    return $change;
	} elsif ($op eq "not" or $op eq "neg") {
	    return $change;
	} elsif ($op =~ /^$shift$/) {
	    return $change;
	} elsif ($op =~ /^set$cond$/) {
	    return $change;
	} elsif ($op =~ /^i?(mul|div)$/) {
	    return $change;
	} elsif ($op =~ /^($fstore|$funary|$fload)$/) {
	    return $change;
	} elsif ($op =~ /^$fcstore$/) {
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($immed)$/) {
	if ($op eq "push") {
	    return USE_ESP|CHANGE_ESP;
	} elsif ($op eq "ret") {
	    return USE_ESP|CHANGE_ESP;
	} else {
	    die "Unknown insn `$op $args' <$1>";
	}
    } elsif ($args =~ /^($complex)$/) {
	my $target = $1;
	my $flags = 0;
	$flags |= USE_ESP if $target =~ /^($lhalf)?\(%esp\)$/;
	if ($op eq "pushl") {
	    return USE_ESP;
	} elsif ($op =~ /^(inc|dec|i?div|i?mul|$shift|neg|not)[bwl]|set$cond|$fstore|$fcstore/) {
	    if ($target eq "(%ebx)" and $safety & $data_safety) {
		return $flags;
	    } elsif ($target =~ /^($lhalf|0xffff[0-9a-f]{4})\(%ebp\)$/) {
		my $offset = signed_word($1);
		if (!($unsafety & CHANGE_EBP) and abs($offset) < 32767) {
		    return 0;
		}
	    } elsif ($target =~ /^($lhalf|0xffff[0-9a-f]{4}|)\(%esp(,1)?\)$/) {
		my $offset = signed_word($1);
		if (!($unsafety & CHANGE_ESP) and abs($offset) < 127) {
		    return 0;
		}
	    }
	    return IWRITE | $flags;
	} elsif ($op =~ /^($fload|$fcload|$fbin|$funary)$/) {
	    return 0;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($immed),($reg),($reg)$/) {
	my($const, $r1, $r2) = ($1, $2, $3);
	my $change;
	$change |= CHANGE_EBP if $r1 =~ /bp/ or $r2 =~ /bp/;
	$change |= CHANGE_ESP if $r1 =~ /sp/ or $r2 =~ /sp/;
	if ($op =~ /^i?(mul|div)l?$/) {
	    return $change;
	} elsif ($op =~ /^($dshift)l?$/) {
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($reg),($reg),($reg)$/) {
	my($r_amt, $r1, $r2) = ($1, $2, $3);
	my $change;
	$change |= CHANGE_EBP if $r1 =~ /bp/ or $r2 =~ /bp/;
	$change |= CHANGE_ESP if $r1 =~ /sp/ or $r2 =~ /sp/;
	if ($op =~ /^($dshift)l?$/) {
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($immed),($complex|$lword),($reg)$/) {
	my($const, $src, $dest) = ($1, $2, $3);
	my $change;
	$change |= CHANGE_EBP if $dest =~ /bp/;
	$change |= CHANGE_ESP if $dest =~ /sp/;
	if ($op =~ /^i(mul|div)l?$/) {
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($reg),($reg)$/) {
	my($from, $to) = ($1, $2);
	my $change;
	$change |= CHANGE_EBP if $to =~ /bp/;
	$change |= CHANGE_ESP if $to =~ /sp/;
	if ($op =~ /^mov([sz](bl|wl|bw))?/) {
	    if ($op eq "mov" and $from eq "%esp" and $to eq "%ebp" and
		not ($unsafety & (CHANGE_ESP|BUMP_ESP))) {
		return $ebp_safety;
	    } elsif ($op eq "mov" and $from eq "%ebp" and $to eq "%esp"
		     and not ($unsafety & CHANGE_EBP)) {
		return USE_ESP;
	    }
	    return $change;
	} elsif ($op =~ /^$arith|$shift|$fbin$/) {
	    return $change;
	} elsif ($op eq "xchg") {
	    # xchg changes both regs...
	    $change |= CHANGE_EBP if $from =~ /bp/;
	    $change |= CHANGE_ESP if $from =~ /sp/;
	    # unless they're the same one
	    $change = 0 if $from eq $to;
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($immed),($reg)$/) {
	my($val, $to) = ($1, $2);
	my $change;
	$change |= CHANGE_EBP if $to =~ /bp/;
	if ($to eq "%esp") {
	    if ($op =~ /^(add|sub)l?$/ and $val =~ /^\$$lbyte/) {
		$change |= BUMP_ESP;
	    } elsif ($op eq "and" and $val eq '$0xfffffff0') {
		$change |= BUMP_ESP;
	    } else {
		$change |= CHANGE_ESP;
	    }
	}
	if ($op eq "mov") {
	    return $change;
	} elsif ($op =~ /^$arith8$/) {
	    if ($op eq "and") {
		if ($args eq "$DATA_MASK,%ebx") {
		    return EBX_DATA_SAFE;
		} elsif ($args eq "$JUMP_MASK,%ebx") {
		    return EBX_CODE_SAFE;
		} elsif ($args eq "$DATA_MASK,%ebp") {
		    return EBP_DATA_SAFE;
		} elsif ($args eq "$DATA_MASK,%esp") {
		    return $esp_safety;
		}
	    } elsif ($op eq "or") {
		if ($args eq "$DATA_START,%ebx"
		    and $safety & EBX_DATA_SAFE) {
		    return EBX_DATA_XSAFE;
		} elsif ($args eq "$CODE_START,%ebx"
		    and $safety & EBX_CODE_SAFE) {
		    return EBX_CODE_XSAFE;
		} elsif ($args eq "$DATA_START,%ebp"
		    and $safety & EBP_DATA_SAFE) {
		    return EBP_DATA_XSAFE;
		} elsif ($args eq "$DATA_START,%esp"
		    and $safety & ESP_DATA_SAFE) {
		    return USE_ESP;
		}
	    }
	    return $change;
	} elsif ($op =~ /^($shift|test)$/) {
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($immed),($complex)$/) {
	my($val, $to) = ($1, $2);
	my $simple = 0;
	if ($to =~ /^($lhalf|0xffff[0-9a-f]{4})\(%ebp\)$/) {
	    my $offset = signed_word($1);
	    if (!($unsafety & CHANGE_EBP) and abs($offset) < 32768) {
		$simple = USE_EBP;
	    }
	} elsif ($to =~ /^($lhalf|0xffff[0-9a-f]{4}|)\(%esp(,1)?\)$/) {
	    my $offset = signed_word($1);
	    if ($op eq "andl" and $val eq $JUMP_MASK
		and ($to = "(%esp,1)" || $to eq "(%esp)")) {
		return USE_ESP|STACK_TOP_SAFE;
	    } elsif ($op eq "orl" and $val eq $CODE_START
		     and ($to = "(%esp,1)" || $to eq "(%esp)")
		     and $safety & STACK_TOP_SAFE) {
		return USE_ESP|STACK_TOP_XSAFE;
	    } elsif (!($unsafety & CHANGE_ESP) and abs($offset) < 127) {
		$simple = USE_ESP;
	    }
	} elsif ($to eq "(%ebx)" and $safety & $data_safety) {
	    return 0;
	}
	if ($op =~ /^mov[bwl]?$/) {
	    return $simple ? $simple : IWRITE;
	} elsif ($op =~ /^(cmp|test)[bwl]$/) {
	    # actually just a read!
	    return $simple ? $simple : IREAD;
	} elsif ($op =~ /^($arith|$shift)[bwl]?$/) {
	    return $simple ? $simple : IWRITE;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($lword)$/) {
	my($haddr) = $1;
	my $addr = hex $1;
	if ($op =~ /^($unary|$shift(?:[bwl])|i?(mul|div)l|$fload|$fbin|$fstore|$funary|set$cond)$/) {
	    die "Bad data address $haddr" unless
	      $addr >= $data_start and $addr < $data_end;
	    if ($op eq "pushl") {
		return USE_ESP;
	    } else {
		return 0;
	    }
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($lword),($reg)$/) {
	my($haddr, $to) = ($1, $2);
	my $change;
	$change |= CHANGE_EBP if $to =~ /bp/;
	$change |= CHANGE_ESP if $to =~ /sp/;
	my $addr = hex $haddr;
	die "Bad data address $haddr" unless
	  $addr >= $data_start and $addr < $data_end;
	return $change;
    } elsif ($args =~ /^($reg|$immed),($lword)$/) {
	my($from, $haddr) = ($1, $2);
	my $addr = hex $haddr;
	die "Bad data address $haddr" unless
	  $addr >= $data_start and $addr < $data_end;
	return 0;
    } elsif ($args =~ /^($complex),($reg)$/) {
	my($from, $to) = ($1, $2);
	my $change;
	$change |= CHANGE_EBP if $to =~ /bp/;
	if ($to =~ /sp/) {
	    if ($op eq "lea" and $from =~ /^(0x[0-9a-f]+)\(%e[bs]p(,1)?\)$/
		and abs(signed_word($1)) < 128) {
		$change |= BUMP_ESP;
	    } else {
		$change |= CHANGE_ESP;
	    }
	}
	my $simple_ebp;
	if ($from =~ /^(0x[0-9a-f]+)\(%ebp\)$/) {
	    my $offset = signed_word($1);
	    if (!($unsafety & CHANGE_EBP) and abs($offset) <= 65535) {
		$simple_ebp = 1;
	    }
	}
	if ($op =~ /^mov([sz](bl|wl|bw))?$/) {
	    return $change | ($simple_ebp ? USE_EBP : IREAD);
	} elsif ($op =~ /^$arith$/) {
	    return $change | ($simple_ebp ? USE_EBP : IREAD);
	} elsif ($op eq "lea") {
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($reg),($complex)$/) {
	my($from, $to) = ($1, $2);
	my $simple = 0;
	if ($to =~ /^(-?0x[0-9a-f]+)\(%ebp\)$/) {
	    my $offset = signed_word($1);
	    if (!($unsafety & CHANGE_EBP) and abs($offset) < 32767) {
		$simple = USE_EBP;
	    }
	} elsif ($to =~ /^((-?0x[0-9a-f]+)?)\(%esp(,1)?\)$/) {
	    my $offset = signed_word($1);
	    if (!($unsafety & CHANGE_ESP) and abs($offset) < 127) {
		$simple = USE_ESP;
	    }
	} elsif ($to eq "(%ebx)" and $safety & $data_safety) {
	    return 0;
	}
	if ($op =~ /^mov[bwl]?$/) {
	    return $simple ? $simple : IWRITE;
	} elsif ($op =~ /^(cmp|test)[bwl]?$/) {
	    # actually just a read!
	    return $simple ? $simple : IREAD;
	} elsif ($op =~ /^$arith$/) {
	    return $simple ? $simple : IWRITE;
	} elsif ($op =~ /^${shift}[bwl]$/) {
	    return $simple ? $simple : IWRITE;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /([0-9a-f]{1,8}) <.*>$/) {
	my $target = hex $1;
	if ($target & ($chunk_size - 1)) {
	    die "Unaligned literal target"
	      unless $op eq "je" and $style eq "test";
	}
	if ($target < $code_start or $target >= $code_end) {
	    die "Literal target out of range";
	}
	my $flags = JUMP;
	$flags |= USE_ESP if $op eq "call";
	return $flags;
    } elsif ($args =~ /^\*($reg)$/) {
	my $target = $1;
	my $flags = JUMP;
	$flags |= USE_ESP if $op eq "call";
	if ($target eq "%ebx" and $safety & $code_safety) {
	    return $flags;
	} else {
	    return IJUMP|$flags;
	}
    } elsif ($args =~ /^%ss:0x0\(%esi(,1)?\),%esi$/) {
	if ($op eq "lea") {
	    # OK, we use this as a weird nop
	    return 0;
	} else {
	    die "Weird args";
	}
    } elsif ($args eq "0x0(%esi,%eiz,1),%esi" or
	     $args eq "0x0(%edi,%eiz,1),%edi") {
	if ($op eq "lea") {
	    # New disassembly of another weird no-op
	    return 0;
	} else {
	    die "Weird args";
	}
    } elsif ($args =~ /^%gs:0x14,($reg)$/) {
	my $target = $1;
	my $change = 0;
	$change |= CHANGE_EBP if $target =~ /bp/;
	$change |= CHANGE_ESP if $target =~ /sp/;
	if ($op eq "mov" or $op eq "xor") {
	    # %gs:0x14 is used by gcc to implement -fstack-protector.
	    # Treat it here like a special-purpose register that can
	    # only be written like this.
	    return $change;
	} else {
	    die "Weird stack protector insn: $op $args";
	}
	return 0;
    } else {
	die "Weird arguments $args in $op\n";
    }
}

my $next_aligned = $code_start;
my $safety;
my $unsafety;
my $bump_count = 0;

while (<>) {
    chomp;
    if (/^([0-9a-f]+):.{22}\t([a-z123]{2,7}) *(.*)$/) {
	my $addr = hex($1);
	my $op = $2;
	my $args = $3;
	$safety = 0 if $addr == $next_aligned;
	if ($unsafety & CHANGE_EBP and $args =~ /%ebp/ and 
	    !($op eq "and" and $args eq "$DATA_MASK,%ebp") and
	    !($op eq "or" and $args eq "$DATA_START,%ebp" )) {
	    printf "Use of unsafe %%ebp at 0x%08x\n", $addr;
	}
	if ($unsafety & CHANGE_ESP and
	    ($args =~ /%esp./ || $op =~ /push|pop|call|ret/)) {
	    printf "Use of unsafe %%esp at 0x%08x\n", $addr;
	}
	if ($op !~ /^(mov(|l|b|w|[sz]bl|[sz]wl|[sz]bw)|lea|$unary(?:b|w|l)?|nop|($shift|$dshift)(?:[bwl])?|$arith(?:b|w|l)?|j$cond|set$cond|jecxz|jmp|call|leave|ret|pushf|popf|$convert|cld|$fload|$fbin|$fstore|$fconst|$funary|$fcstore|$fcload|sahf|xchg|int3)$/) {
	    die "Unknown opcode $op";
	}
	my $flags = check_insn($op, $args, $safety, $unsafety);
	if ($flags & (IJUMP|IWRITE)) {
	    printf "Unsafe %s %s at 0x%08x ($safety)\n", $op, $args, $addr;
	}
	if ($flags & JUMP and $unsafety & CHANGE_EBP) {
	    printf "Unsafe %%ebp escapes by jump at 0x%08x\n", $addr;
	}
	if ($flags & JUMP and $unsafety & (CHANGE_ESP|BUMP_ESP)) {
	    printf "Unsafe %%esp escapes by jump at 0x%08x\n", $addr;
	}
	if ($op eq "popf") {
	    $safety = $safety & (EBX_DATA_SAFE|EBX_CODE_SAFE);
	} else {
	    $safety = 0;
	}
	$safety |= $flags & (EBX_DATA_SAFE|EBX_CODE_SAFE|STACK_TOP_SAFE|
			     EBX_DATA_XSAFE|EBX_CODE_XSAFE|STACK_TOP_XSAFE|
			     EBP_DATA_SAFE|ESP_DATA_SAFE);
	$unsafety |= $flags & (CHANGE_EBP|CHANGE_ESP|BUMP_ESP);
	$bump_count++ if $flags & BUMP_ESP;
	$unsafety |= CHANGE_ESP if $bump_count >= 250;
	$unsafety &= ~(CHANGE_ESP|BUMP_ESP) if $flags & USE_ESP;
	$bump_count = 0 if $flags & USE_ESP;
	$unsafety &= ~CHANGE_EBP if $flags & $ebp_safety;
	if ($addr > $next_aligned) {
	    printf "Missing instruction at %08x\n", $next_aligned;
	    die;
	} elsif ($addr == $next_aligned) {
	    $next_aligned += $chunk_size;
	}
    }
}
printf "Checks finished before %08x\n", $next_aligned;
