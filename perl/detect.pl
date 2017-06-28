#!/usr/local/bin/perl -w

# The function in this program, DetectJPEncoding(), checks the data that
# it is given, and returns various values depending on what encoding it
# detected. The return values are listed in the definiton of the %codes
# hash below. You can feed this function as much data as you wish (such
# as single characters, whole lines, or entire buffer), but the more you
# give to it, the better the chance it will correctly return a single
# encoding (that is, not "Ambiguous"). It currently deals with Japanese
# encodings.

%codes = (
  0 => "ERROR",
  1 => "Shift-JIS",
  2 => "EUC-JP",
  3 => "Ambiguous",   # Means ASCII, Shift-JIS, or EUC-JP
  4 => "ISO-2022-JP"
);

open(SJS,"<jis.sjs") || die "Cannot open Shift-JIS file!\n";
open(EUC,"<jis.euc") || die "Cannot open EUC-JP file!\n";
open(JIS,"<jis.jis") || die "Cannot open ISO-2022-JP file!\n";
open(OUT,">out") || die "Cannot open output file!\n";

# $sjs_data = <SJS>; close SJS;
# $euc_data = <EUC>; close EUC;
# $jis_data = <JIS>; close JIS;

while (defined($line = <SJS>)){
  print OUT $codes{&DetectJPEncoding($line)} . ": " . $line;
}
close(SJS);

while (defined($line = <EUC>)){
  print OUT $codes{&DetectJPEncoding($line)} . ": " . $line;
}
close(EUC);

while (defined($line = <JIS>)){
  print OUT $codes{&DetectJPEncoding($line)} . ": " . $line;
}
close(JIS);

sub DetectJPEncoding ($) {
  my $data = shift;
  return 4 if $data =~ m{ # Return from subroutine if ISO-2022-JP
    \e            # Escape character
    (?:
        \$[\@B]   # JIS X 0208 series
      | \([BHIJ]  # ASCII or JIS X 0201-1997
    )
  }x;
  my ($sjs_out,$euc_out) = (0,0);
  my $euc = qq< # EUC-JP encoding
    [\\x00-\\x7F]                     # Code set 0
    | \\x8E[\\xA0-\\xDF]              # Code set 2
    | \\x8F[\\xA1-\\xFE][\\xA1-\\xFE] # Code set 3
    | [\\xA1-\\xFE][\\xA1-\\xFE]      # Code set 1
  >;
  my $sjs = qq< # Shift-JIS encoding
    [\\x00-\\x7F\\xA0-\\xDF]          # ASCII and half-width katakana
    | [\\x81-\\x9F\\xE0-\\xFC][\\x40-\\x7E\\x80-\\xFC] # Two-byte range
  >;
  $sjs_out = 1 if $data =~ /\A (?:$sjs)+ \Z/ox;
  $euc_out = 2 if $data =~ /\A (?:$euc)+ \Z/ox;

  return ($sjs_out + $euc_out);
}