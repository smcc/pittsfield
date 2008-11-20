package x86_common;
require Exporter;
our @ISA = "Exporter";
our @EXPORT = qw($ereg $ereg_no_esp $breg $freg
		 $arith8 $arith8l $arith $arithl
		 $shift $dshift $unary $convert
		 $fbin $fload $fstore $fconst $funary $fcstore $fcload
		 $cond $label
		 $b_sign $eb_off $e_off $imb_sign $any_const
		 $lbyte $lhalf $lword $reg $immed $complex $lab_complex);

our $ereg = qr/%(?:e[abcd]x|e[sd]i|esp|ebp)/;
our $ereg_no_esp = qr/%(?:e[abcd]x|e[sd]i|ebp)/;
our $wreg = qr/%(?:[abcd]x|[sd]i|sp|bp)/;
our $breg = qr/%(?:[abcd][lh])/;
our $freg = qr/%st(?:\([0-7]\))?/;

our $arith8 = qr/(?:add|adc|and|xor|or|sbb|sub|cmp)/;
our $arith8l = qr/${arith8}l/;
our $ariths = qr/(?:i?mul|i?div)/;
our $arith = qr/(?:$arith8|$ariths|test|bs[fr])/;
our $arithl = qr/${arith}l/;
our $shift = qr/(?:rol|ror|rcl|rcr|shl|sal|shr|sar)/;
our $dshift = qr/$shift(?:d)/;
our $unary = qr/(?:pushl?|popl?|inc(?:b|w|l|)|dec(?:b|w|l|)|not|neg)/;
our $convert = qr/(?:cwtl|cltd|cbtw)/;

our $fbin = qr/(?:f(?:add|mul|divr?|subr?)(?:s|l|)p?)/;
our $fstore = qr/(?:fi?stp?(?:s|t|l|ll|))/;
our $fload = qr/(?:fi?(?:add|mul|subr?|divr?|comp?|ld)(?:s|t|l|ll|))/;
our $fconst = qr/fld(?:1|l2t|l2e|pi|lg2|ln2|z)/;
our $funary = qr/f(?:xch|abs|chs|cos|sin|sincos|sqrt|comp{0,2}(?:s|l|)|ucomp{0,2}|scale|tst|rndint|yl2x|patan|2xm1)/;
our $fcstore = qr/fn?st[sc]w/;
our $fcload = qr/fldcw/;

our $cond = qr/n?(?:o|b|c|ae|z|e|be|a|s|p|pe|po|l|ge|le|g)/;
our $label = qr/\.L(?:RWSOPS)?\d+/;

our $b_hex = qr/0x[0-9a-f][0-9a-f]?/;
our $h_hex = qr/0x[0-9a-f]{1,4}/;
our $w_hex = qr/0x[0-9a-f]{1,8}/;
our $b_sign = qr/-128|-?(?:12[0-7]|1[01]\d|\d\d|\d)|-?(?:$b_hex)/;
our $h_sign = qr/-32768|-?(?:3276[0-7]|327[0-5]\d|32[0-6]\d\d|3[01]\d{3}|\d{1,4})|-?(?:$h_hex)/;
our $w_sign = qr/-?(?:\d+|$w_hex)/;
our $eb_off = qr/$b_sign\($ereg_no_esp\)/;
our $e_off = qr/$w_sign\($ereg_no_esp\)/;
our $imb_sign = qr/\$$b_sign/;

our $lbyte = qr/(?:$b_sign)/;
our $lhalf = qr/(?:$h_sign)/;
our $lword = qr/(?:$w_sign)/;
our $any_const = qr/(?:$lbyte|$lword|.L\w+|[\w+\.-]+)(?:[+-]\d+)?/;
our $immed = qr/\$$any_const/;
our $reg = qr/$ereg|$wreg|$breg|$freg/;
our $complex = qr/(?:$lbyte|$lword)?\((?:$ereg)?(?:,$ereg)?(?:,[1248])?\)/;
our $lab_complex = qr/(?:$any_const)?\((?:$ereg)?(?:,$ereg)?(?:,[1248])?\)/;

1;
