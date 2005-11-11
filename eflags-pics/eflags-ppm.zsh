#!/bin/zsh
perl eflags-heuristic.pl -safer $1 >/tmp/red.bits
perl eflags-heuristic.pl        $1 >/tmp/green.bits
perl eflags-liveness.pl         $1 >/tmp/blue.bits

perl bits2ppm-hilbert.pl r /tmp/red.bits   >/tmp/red.ppm
perl bits2ppm-hilbert.pl g /tmp/green.bits >/tmp/green.ppm
perl bits2ppm-hilbert.pl b /tmp/blue.bits  >/tmp/blue.ppm

pnmarith -add /tmp/red.ppm  <(pnmarith -add /tmp/green.ppm /tmp/blue.ppm) >/tmp/img.ppm

paste $1 <(perl bits2colors.pl /tmp/red.bits /tmp/green.bits /tmp/blue.bits | tail +2) >/tmp/annotated.s

rm /tmp/red.bits /tmp/green.bits /tmp/blue.bits
rm /tmp/red.ppm /tmp/green.ppm /tmp/blue.ppm



