#!/usr/local/bin/perl -w

# This program repairs damaged ISO-2022-JP-encoded files by correctly
# restoring stripped or otherwise mangled escape characters. Escape
# characters can be mangled as follows:
#
# o Converted into a single space (0x20)
# o Converted into URL encoding -- "%1B"
# o Converted into quoted-printable -- "=1B"
#
# Or they are simply deleted.

while (defined($line = <STDIN>)) {
  $line =~ s{
    (?:\x20|[=%]1[Bb])?               # Optional space or encoded escape 
    (
      (?:
        \$ [\@B]                      # Dollar sign plus @ or B
        (?:[\x21-\x7E][\x21-\x7E])+   # One or more two-byte characters
      )
      |                               # Or...
      (?:
        \( I                          # Open parenthesis plus I
        [\x20-\x5F]+?                 # One or more half-width katakana
      )
    )
    (?:\x20|[=%]1[Bb])?               # Optional space or encoded escape 
    (
      \( [BHJ]                        # Opening parenthesis plus B, H, or J
    )
  }{\e$1\e$2}gx;
  print STDOUT $line;
}
