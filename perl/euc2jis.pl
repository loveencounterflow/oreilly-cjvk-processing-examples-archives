#!/usr/local/bin/perl -w

# EUC-JP to ISO-2022-JP

while (defined($line = <STDIN>)) {
  $line =~ s{ # JIS X 0208:1997
    ((?:[\xA1-\xFE][\xA1-\xFE])+)
  }{\e\$B$1\e\(J}gx;
  $line =~ s{ # JIS X 0201-1997 half-width katakana
    ((?:\x8E[\xA0-\xDF])+)          # Half-width katakana
  }{\e\(I$1\e\(J}gx;
  $line =~ s/\x8E//g;               # Remove SS2s
  $line =~ tr/\xA1-\xFE/\x21-\x7E/; # From 8- to 7-bit
  print STDOUT $line;
}
