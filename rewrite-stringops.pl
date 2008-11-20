#!/usr/bin/perl

use strict;

my $label_count = 1;

while (<>) {
    if (/^\t(rep(?:e|ne|z|nz|))(?: (\w+))?$/) {
	my $repop = $1;
	my $stringop;
	if ($2) {
	    $stringop = $2;
	} else {
	    $stringop = <>;
	    chomp $stringop;
	    $stringop =~ s/^\t//;
	}
	my $head_label = ".LRWSOPS$label_count"; $label_count++;
	my $tail_label = ".LRWSOPS$label_count"; $label_count++;
	my $save_eax = ($stringop !~ /^stos/);
	print "\tpushl\t%eax\n" if $save_eax;
	print "$head_label:\n";
	print "\tjecxz\t$tail_label\n";
	if ($stringop eq "movsl") {
	    # strcpy
	    print "\tmovl\t(%esi), %eax\n";
	    print "\tmovl\t%eax, (%edi)\n";
	    print "\tleal\t4(%esi), %esi\n";
	    print "\tleal\t4(%edi), %edi\n";
	    print "\tleal\t-1(%ecx), %ecx\n";
	    print "\tjmp\t$head_label\n";
	} elsif ($stringop eq "stosl") {
	    # memset/memzero
	    print "\tmovl\t%eax, (%edi)\n";
	    print "\tleal\t4(%edi), %edi\n";
	    print "\tleal\t-1(%ecx), %ecx\n";
	    print "\tjmp\t$head_label\n";
	} elsif ($repop eq "repz" and $stringop eq "cmpsb") {
	    # strncmp
	    print "\tmovb\t(%esi), %al\n";
	    print "\tcmpb\t%al, (%edi)\n";
	    print "\tleal\t1(%esi), %esi\n";
	    print "\tleal\t1(%edi), %edi\n";
	    print "\tleal\t-1(%ecx), %ecx\n";
	    print "\tjz\t$head_label\n";
	} else {
	    die "Unsupported string op $repop $stringop\n";
	}
	print "$tail_label:\n";
	print "\tpopl\t%eax\n" if $save_eax;
    } elsif (/^\tcld/) {
	next;
    } elsif (/^\tstd/) {
	die "Backwards string ops are not supported";
    } else {
	print;
    }
}
