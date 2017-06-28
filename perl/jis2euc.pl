#!/usr/local/bin/perl -w

# ISO-2022-JP to EUC-JP

while (defined($line = <STDIN>)) {
  $line =~ s{ # JIS X 0208:1997
    \e\$[\@B]                     # ESC $ plus @ or B
    ((?:[\x21-\x7E][\x21-\x7E])+) # Two-byte characters
  }{($x = $1) =~ tr/\x21-\x7E/\xA1-\xFE/, # From 7- to 8-bit
    $x
  }egx;
  $line =~ s{ # JIS X 0201-1997 half-width katakana
    \e\(I          # ESC ( I
    ([\x21-\x7E]+) # Half-width katakana
  }{($x = $1) =~ tr/\x21-\x7E/\xA1-\xFE/,  # From 7- to 8-bit
    ($y = $x) =~ s/([\xA1-\xFE])/\x8E$1/g, # Prefix with SS2
    $y
  }egx;
  $line =~ s/\e\([BHJ]//g;
  print STDOUT $line;
}
