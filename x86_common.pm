package x86_common;
require Exporter;
our @ISA = "Exporter";
our @EXPORT = qw($ereg $breg
		 $arith8 $arith8l $arith $arithl
		 $shift $unary
		 $cond $label
		 $b_sign $eb_off $imb_sign
		 $lbyte $lhalf $lword $reg $immed $complex $lab_complex);

our $ereg = qr/%(?:e[abcd]x|e[sd]i|esp|ebp)/;
our $wreg = qr/%(?:[abcd]x|[sd]i|sp|bp)/;
our $breg = qr/%(?:[abcd][lh])/;

our $arith8 = qr/(?:add|adc|and|xor|or|sbb|sub|cmp)/;
our $arith8l = qr/${arith8}l/;
our $ariths = qr/(?:i?mul|i?div)/;
our $arith = qr/(?:$arith8|$ariths|test)/;
our $arithl = qr/${arith}l/;
our $shift = qr/(?:rol|ror|rcl|rcr|shl|sal|shr|sar)/;
our $unary = qr/(?:pushl?|popl?|incl?|decl?|not|neg)/;

our $cond = qr/n?(?:o|b|c|ae|z|e|be|a|s|p|pe|po|l|ge|le|g)/;
our $label = qr/\.L\d+/;

our $b_sign = qr/-128|-?(?:12[0-7]|1[01]\d|\d\d|\d)/;
our $h_sign = qr/-32768|-?(?:3276[0-7]|327[0-5]\d|32[0-6]\d\d|3[01]\d{3}|\d{1,4})/;
our $w_sign = qr/-?\d+/;
our $b_hex = qr/0x[0-9a-f][0-9a-f]?/;
our $h_hex = qr/0x[0-9a-f]{1,4}/;
our $w_hex = qr/0x[0-9a-f]{1,8}/;
our $eb_off = qr/$b_sign\($ereg\)/;
our $imb_sign = qr/\$$b_sign/;

our $lbyte = qr/(?:$b_sign|$b_hex)/;
our $lhalf = qr/(?:$h_sign|$h_hex)/;
our $lword = qr/(?:$w_sign|$w_hex)/;
our $any_const = qr/(?:$lbyte|$lword|.L\w+|\w+)/;
our $immed = qr/\$$any_const/;
our $reg = qr/$ereg|$wreg|$breg/;
our $complex = qr/(?:$lbyte|$lword)?\((?:$ereg)?(?:,$ereg)?(?:,[1248])?\)/;
our $lab_complex = qr/(?:$any_const)?\((?:$ereg)?(?:,$ereg)?(?:,[1248])?\)/;

1;
