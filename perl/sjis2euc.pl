#!/usr/local/bin/perl -w

# Shift-JIS to EUC-JP

sub sjis2jis { # For Shift-JIS to ISO-2022-JP and EUC-JP
  my @ord = unpack("C*", $_[0]);
  for ($i = 0; $i < @ord; $i += 2) {
    $ord[$i] = (($ord[$i]-($ord[$i]<160?112:176))<<1)-
      ($ord[$i+1]<159?1:0);
    $ord[$i+1] -= ($ord[$i+1]<159?($ord[$i+1]>127?32:31):126);
  }
  return pack("C*", @ord);
}

while (defined($line = <STDIN>)) {
  $line =~ s{( # JIS X 0208:1997 and half-width katakana
    (?:[\x81-\x9F\xE0-\xEF][\x40-\x7E\x80-\xFC])+|
    [\xA0-\xDF]+
  )}{
    ($x = $1) !~ /^[\xA0-\xDF]/ ?
    (($y = &sjis2jis($x)) =~ tr/\x21-\x7E/\xA1-\xFE/, $y) :
    (($y = $x) =~ s/([\xA0-\xDF])/\x8E$1/g, $y)
  }egx;
  print STDOUT $line;
}
