#!/usr/local/bin/perl -w

# This program shows how to break up multiple-byte text into
# separate array elements; it prints every character, one per
# line; two-byte characters as hexadecimal with "0x" prefix

$encoding = qq< # Shift-JIS encoding
  [\x00-\x80\xFD-\xFF]|       # ASCII and other one-byte
  [\xA0-\xDF]|                # Half-width katakana
  [\x81-\x9F\xE0-\xFC][\x40-\x7E\x80-\xFC] # Two-byte range
>;

while (defined($line = <STDIN>)) {
  @enc = $line =~ /($encoding)/gox; # One character per element
  foreach $element (@enc) {
    if (length($element) == 2) { # If two-byte character
      print STDOUT "0x" . ($x = uc unpack("H*",$element), $x);
    } else { # All others are one-byte characters
      print STDOUT "$element\n";
    }
  }
}
