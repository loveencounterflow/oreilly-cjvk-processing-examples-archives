#!/usr/local/bin/perl -w

# ISO-2022-JP or EUC-JP to Shift-JIS

sub convert2sjis { # For EUC-JP and ISO-2022-JP to Shift-JIS
  my @euc = unpack("C*", $_[0]);
  my @out = ();
  while (($hi, $lo) = splice(@euc, 0, 2)) {
    $hi &= 0x7f; $lo &= 0x7f;
    push(@out, (($hi + 1) >> 1) + ($hi < 95 ? 112 : 176),
      $lo + (($hi & 1) ? ($lo > 95 ? 32 : 31) : 126));
  }
  return pack("C*", @out);
}

while (defined($line = <STDIN>)) {
  $line =~ s{( # EUC-JP
    (?:[\xA1-\xFE][\xA1-\xFE])+| # JIS X 0208:1997
    (?:\x8E[\xA0-\xDF])+         # Half-width katakana
  )}{substr($1,0,1) eq "\x8E" ? (($x = $1) =~ s/\x8E//g, $x) :
     &convert2sjis($1)}egx;
  $line =~ s{ # Handle ISO-2022-JP
    \e\$[\@B]
    ((?:[\x21-\x7E][\x21-\x7E])+)
    \e\([BHJ]
  }{&convert2sjis($1)}egx;
  $line =~ s{ # Handle ISO-2022-JP half-width katakana
    \e\(I
    ([\x20-\x5F]+)
    \e\([BHJ]
  }{($x = $1) =~ tr/\x20-\x5F/\xA0-\xDF/, $x}egx;
  print STDOUT $line;
}
