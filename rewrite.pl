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
        return 4;
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
    if ($args =~ /^($immed|$reg), ($lab_complex)$/) {
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
	}
	my $size = "l";
	$size = $1 if $op =~ /([bwl])$/;
	print "\tleal\t$to, %ebx\n";
	align();
	print "\tandl\t\$0x20ffffff, %ebx\n";
	print "\t$op\t$from, (%ebx)\n";
	align();
	return 1;
    } elsif ($args =~ /^$complex$/) {
	my $target = $args;
	return 0 if $op =~ /^lea[blw]$/;
	if ($target =~ /^(-?\d+)\(%e[bs]p\)$/) {
	    my($offset, $base_reg) = ($1, $2);
	    if ($base_reg eq "%ebp" and  abs($offset) < 32768) {
		return 0;
	    } elsif ($base_reg eq "%esp" and abs($offset) < 127) {
		return 0;
	    }
	}
	print "\tleal\t$target, %ebx\n";
	align();
	print "\tandl\t\$0x20ffffff, %ebx\n";
	print "\t$op\t(%ebx)\n";
	$this_chunk = 8;
	return 1;
    } elsif ($op eq "ret") {
	die if $args;
	print "\tpopl\t%ebx\n";
	align();
	$dirty_esp = 0;
	print "\tandl\t\$0x10fffff0, %ebx\n";
	print "\tjmp\t*%ebx\n";
	$this_chunk = 8;
	return 1;
    } elsif ($op eq "call") {
	my $real_call;
	my $call_len;
	if ($args =~ /^\.?\w+$/) {
	    $real_call = $line;
	    $call_len = 5;
	} elsif ($args =~ /^\*($complex)$/) {
	    my $target = $1;
	    print "\tmovl\t$target, %ebx\n";
	    $real_call = "\tandl\t\$0x10fffff0, %ebx\n";
	    $real_call .= "\tcall\t*%ebx\n";
	    $call_len = 8; # andl; FF 13
	} elsif ($args =~ /^\*($reg)$/) {
	    my $target = $1;
	    print "\tmovl\t$target, %ebx\n";
	    $real_call = "\tandl\t\$0x10fffff0, %ebx\n";
	    $real_call .= "\tcall\t*%ebx\n";
	    $call_len = 8; # andl; FF 13
	}
	align();
	if ($dirty_esp) {
	    nop_pad(16 - 6 - $call_len);
	    print "\tandl\t\$0x20ffffff, %esp\n";
	} else {
	    nop_pad(16 - $call_len);
	}
	print "$real_call\n";
	$dirty_esp = 0;
	return 1;
    } elsif ($op eq "jmp") {
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
	print "\tandl\t\$0x10fffff0, %ebx\n";
	print "\tjmp\t*%ebx\n";
	$this_chunk = 8;
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
nop_pad(11);
print "\tcall main\n"; # 5 bytes
print "\tret\n";
print "\t.p2align 4\n";

open(STUBS, "<stub-list");
my $i = 0;
while (<STUBS>) {
    my $f = $_;
    chomp $f;
    print "$f:\n";
    printf "\tjmp\t0x100%04x0\n", $i;
    print "\tpopl\t%ebx\n";
    print "\tandl\t\$0x10fffff0, %ebx\n";
    print "\tjmp\t*%ebx\n";
    print "\t.p2align 4\n";
}
close STUBS;

while (<>) {
    if (/^\s+\.section\s+.rodata/) {
	print ".data\n";
	next;
    } elsif (/^\s+\.section\s+\.note\.GNU-stack/) {
	next;
    }
    if ($dirty_esp and /^\t(jmp|cmp|dec|or|test)/) {
 	align() if $this_chunk + 6 > 16;
 	print "\tandl\t\$0x20ffffff, %esp\n";
 	$this_chunk += 6;
	$dirty_esp = 0;
    }
    if (/^\t(push|pop)/) {
	$dirty_esp = 0;
    } elsif (/, %esp/) {
	$dirty_esp = 1;
    }
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
    if (/\t(leave|popl\t%ebp)$/) {
	align() if $this_chunk + 6 > 16;
	print "\tandl\t\$0x20ffffff, %ebp\n";
	$this_chunk += 6;
    } elsif (/^\tsubl\t%eax, %esp$/) {
	align() if $this_chunk + 6 > 16;
	print "\tandl\t\$0x20ffffff, %esp\n";
	$this_chunk += 6;	
    }
    #print "# <$this_chunk>\n";
}
