use strict;

use x86_common;

sub USE_ESP () { 1 }
sub IJUMP () { 2 }
sub IWRITE () { 4 }
sub IREAD ()  { 8 }
sub USE_EBP () { 16 }
sub CHANGE_EBP () { 32 }
sub EBX_DATA_SAFE () { 64 }
sub EBX_CODE_SAFE () { 128 }
sub JUMP () { 256 }
sub EBP_DATA_SAFE () { 512 }
sub CHANGE_ESP () { 1024 }
sub BUMP_ESP () { 2048 }

sub check_insn {
    my($op, $args, $safety, $unsafety) = @_;
    if ($args eq "") {
	if ($op eq "nop") {
	    return 0;
	} elsif ($op eq "ret") {
	    return JUMP|IJUMP|USE_ESP|CHANGE_ESP;
	} elsif ($op eq "leave") {
	    # move %ebp, %esp; pop %ebp
	    return USE_ESP|CHANGE_EBP|CHANGE_ESP;
	} else {
	    die "Unknown none-ary insn $op";
	}
    } elsif ($args =~ /^($reg)$/) {
	my $target = $1;
	my $change = 0;
	$change |= CHANGE_EBP if $target eq "%ebp";
	$change |= CHANGE_ESP if $target eq "%esp";
	if ($op eq "push") {
	    return USE_ESP|CHANGE_ESP;
	} elsif ($op eq "pop") {
	    return USE_ESP|$change|CHANGE_ESP;
	} elsif ($op eq "inc" or $op eq "dec") {
	    return $change;
	} elsif ($op eq "not" or $op eq "neg") {
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^$immed$/) {
	if ($op eq "push") {
	    return USE_ESP|CHANGE_ESP;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($complex)$/) {
	my $target = $1;
	my $flags = 0;
	$flags |= USE_ESP if $target =~ /^($lhalf)?\(%esp\)$/;
	if ($op eq "pushl") {
	    return USE_ESP;
	} elsif ($op =~ /^(inc|dec|div|mul)l$/) {
	    if ($target eq "(%ebx)" and $safety & EBX_DATA_SAFE) {
		return $flags;
	    } elsif ($target =~ /^($lhalf|0xffff[0-9a-f]{4})\(%ebp\)$/) {
		my $offset = unpack("i", pack("I", hex($1)));
		if (abs($offset) < 32767) {
		    return 0;
		}
	    } elsif ($target =~ /^($lhalf|0xffff[0-9a-f]{4}|)\(%esp,1\)$/) {
		my $offset = unpack("i", pack("I", hex($1)));
		if (abs($offset) < 127) {
		    return 0;
		}
	    }
	    return IWRITE | $flags;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($immed),($reg),($reg)$/) {
	my($const, $r1, $r2) = ($1, $2, $3);
	my $change;
	$change |= CHANGE_EBP if $r1 eq "%ebp" or $r2 eq "%ebp";
	$change |= CHANGE_ESP if $r1 eq "%esp" or $r2 eq "%esp";
	if ($op =~ /^i?(mul|div)l?$/) {
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($reg),($reg)$/) {
	my($from, $to) = ($1, $2);
	my $change;
	$change |= CHANGE_EBP if $to eq "%ebp";
	$change |= CHANGE_ESP if $to eq "%esp";
	if ($op eq "mov") {
	    if ($from eq "%esp" and $to eq "%ebp" and
		not ($unsafety & (CHANGE_ESP|BUMP_ESP))) {
		return EBP_DATA_SAFE;
	    } elsif ($from eq "%ebp" and $to eq "%esp"
		     and not ($unsafety & CHANGE_EBP)) {
		return USE_ESP;
	    }
	    return $change;
	} elsif ($op =~ /^$arith$/) {
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($immed),($reg)$/) {
	my($val, $to) = ($1, $2);
	my $change;
	$change |= CHANGE_EBP if $to eq "%ebp";
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
	    if ($op eq "and" and $args eq '$0x20ffffff,%ebx') {
		return EBX_DATA_SAFE;
	    } elsif ($op eq "and" and $args eq '$0x10fffff0,%ebx') {
		return EBX_CODE_SAFE;
	    } elsif ($op eq "and" and $args eq '$0x20ffffff,%ebp') {
		return EBP_DATA_SAFE;
	    } elsif ($op eq "and" and $args eq '$0x20ffffff,%esp') {
		return USE_ESP;
	    }
	    return $change;
	} elsif ($op =~ /^$shift$/) {
	    return $change;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($immed),($complex)$/) {
	my($val, $to) = ($1, $2);
	my $simple = 0;
	if ($to =~ /^($lhalf|0xffff[0-9a-f]{4})\(%ebp\)$/) {
	    my $offset = unpack("i", pack("I", hex($1)));
	    if (abs($offset) < 32768) {
		$simple = USE_EBP;
	    }
	} elsif ($to =~ /^($lhalf|0xffff[0-9a-f]{4}|)\(%esp,1\)$/) {
	    my $offset = unpack("i", pack("I", hex($1)));
	    if (abs($offset) < 127) {
		$simple = USE_ESP;
	    }
	} elsif ($to eq "(%ebx)" and $safety & EBX_DATA_SAFE) {
	    return 0;
	}
	if ($op =~ /^mov[bwl]?$/) {
	    return $simple ? $simple : IWRITE;
	} elsif ($op =~ /^($arith|$shift)l?$/) {
	    return $simple ? $simple : IWRITE;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($lword)$/) {
	my($haddr) = $1;
	my $addr = hex $1;
	if ($op eq "pushl") {
	    die "Bad data address $haddr" unless
	      $addr >= 0x20000000 and $addr < 0x21000000;
	    return USE_ESP;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /^($lword),($reg)$/) {
	my($haddr, $to) = ($1, $2);
	my $change;
	$change |= CHANGE_EBP if $to eq "%ebp";
	$change |= CHANGE_ESP if $to eq "%esp";
	my $addr = hex $haddr;
	die "Bad data address $haddr" unless
	  $addr >= 0x20000000 and $addr < 0x21000000;
	return $change;
    } elsif ($args =~ /^($reg|$immed),($lword)$/) {
	my($from, $haddr) = ($1, $2);
	my $addr = hex $haddr;
	die "Bad data address $haddr" unless
	  $addr >= 0x20000000 and $addr < 0x21000000;
	return 0;
    } elsif ($args =~ /^($complex),($reg)$/) {
	my($from, $to) = ($1, $2);
	my $change;
	$change |= CHANGE_EBP if $to eq "%ebp";
	if ($to eq "%esp") {
	    if ($op eq "lea" and $from =~ /^(0x[0-9a-f]+)\(%ebp\)$/
		and abs(unpack("i", pack("I", hex($1)))) < 128) {
		$change |= BUMP_ESP;
	    } else {
		$change |= CHANGE_ESP;
	    }
	}
	my $simple_ebp;
	if ($from =~ /^(0x[0-9a-f]+)\(%ebp\)$/) {
	    my $offset = unpack("i", pack("I", hex($1)));
	    if (abs($offset) <= 65535) {
		$simple_ebp = 1;
	    }
	}
	if ($op =~ /^mov([bwl]|z([bw]l)|s([bw]l)|)$/) {
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
	if ($to =~ /^(0x[0-9a-f]+)\(%ebp\)$/) {
	    my $offset = unpack("i", pack("I", hex($1)));
	    if (abs($offset) < 32767) {
		$simple = USE_EBP;
	    }
	} elsif ($to =~ /^((0x[0-9a-f]+)?)\(%esp,1\)$/) {
	    my $offset = unpack("i", pack("I", hex($1)));
	    if (abs($offset) < 127) {
		$simple = USE_ESP;
	    }
	} elsif ($to eq "(%ebx)" and $safety & EBX_DATA_SAFE) {
	    return 0;
	}
	if ($op =~ /^mov[bwl]?$/) {
	    return $simple ? $simple : IWRITE;
	} elsif ($op =~ /^$arith$/) {
	    return $simple ? $simple : IWRITE;
	} else {
	    die "Unknown insn `$op $args'";
	}
    } elsif ($args =~ /([0-9a-f]{1,8}) <.*>$/) {
	my $target = hex $1;
	die "Unaligned literal target" if $target & 15;
	if ($target < 0x1000000 or $target >= 0x11000000) {
	    die "Literal target out of range";
	}
	my $flags = JUMP;
	$flags |= USE_ESP if $op eq "call";
	return $flags;
    } elsif ($args =~ /^\*($reg)$/) {
	my $target = $1;
	my $flags = JUMP;
	$flags |= USE_ESP if $op eq "call";
	if ($target eq "%ebx" and $safety & EBX_CODE_SAFE) {
	    return $flags;
	} else {
	    return IJUMP|$flags;
	}
    } else {
	die "Weird arguments $args in $op\n";
    }
}

my $next_aligned = 0x10000000;
my $safety;
my $unsafety;
my $bump_count = 0;

while (<>) {
    chomp;
    if (/^([0-9a-f]+):.{22}\t([a-z]{2,7}) +(.*)$/) {
	my $addr = hex($1);
	my $op = $2;
	my $args = $3;
	$safety = 0 if $addr == $next_aligned;
	if ($unsafety & CHANGE_EBP and $args =~ /%ebp/ and 
	    not($op eq "and" and $args eq '$0x20ffffff,%ebp')) {
	    printf "Use of unsafe %%ebp at 0x%08x\n", $addr;
	}
	if ($unsafety & CHANGE_ESP and
	    ($args =~ /%esp./ || $op =~ /push|pop|call|ret/)) {
	    printf "Use of unsafe %%esp at 0x%08x\n", $addr;
	}
	if ($op !~ /^(mov(|l|b|w|[sz]bl|[sz]wl)|lea|$unary|nop|$shift(?:l)?|$arith(?:l)?|j$cond|jmp|call|leave|ret)$/) {
	    die "Unknown opcode $op";
	}
	my $flags = check_insn($op, $args, $safety, $unsafety);
	if ($flags & (IJUMP|IWRITE) and $addr != 0x10000010) {
	    printf "Unsafe %s %s at 0x%08x ($safety)\n", $op, $args, $addr;
	}
	if ($flags & JUMP and $unsafety & CHANGE_EBP) {
	    printf "Unsafe %%ebp escapes by jump at 0x%08x\n", $addr;
	}
	if ($flags & JUMP and $unsafety & (CHANGE_ESP|BUMP_ESP)) {
	    printf "Unsafe %%esp escapes by jump at 0x%08x\n", $addr;
	}
	$safety |= $flags & (EBX_DATA_SAFE|EBX_CODE_SAFE);
	$unsafety |= $flags & (CHANGE_EBP|CHANGE_ESP|BUMP_ESP);
	$bump_count++ if $flags & BUMP_ESP;
	$unsafety |= CHANGE_ESP if $bump_count >= 250;
	$unsafety &= ~(CHANGE_ESP|BUMP_ESP) if $flags & USE_ESP;
	$bump_count = 0 if $flags & USE_ESP;
	$unsafety &= ~CHANGE_EBP if $flags & EBP_DATA_SAFE;
	if ($addr > $next_aligned) {
	    printf "Missing instruction at %08x\n", $next_aligned;
	    die;
	} elsif ($addr == $next_aligned) {
	    $next_aligned += 16;
	}
    }
}
printf "Checks finished before %08x\n", $next_aligned;
