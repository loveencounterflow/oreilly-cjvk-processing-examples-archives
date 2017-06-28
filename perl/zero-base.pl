#!/usr/local/bin/perl -w

# Converting EUC-JP code set 1 to zero-based values

$ch = "\xB7\xF5"; # The kanji A of JIS X 0208:1997

# Subtract 0xA1 (161) from the first byte then multiple by 94
# Subtract 0xA1 (161) from the second byte
# Add the two values to obtain zero-based value

$zeroch = ((ord(substr($ch,0,1)) - 0xA1) * 94) +
  (ord(substr($ch,1,1)) - 0xA1);
print "Zero-based value of $ch is $zeroch\n";
# $zeroch equals 2152 -- the 2,153rd character

# The following converts the zero-based value back to the original
# by reversing the effects of zero-based conversion

$ch = chr(($zeroch / 94) + 0xA1) . chr(($zeroch % 94) + 0xA1);
print "$ch\n";
# $ch again equals 0xB7F5 (A)
