#!/usr/local/bin/perl -w

# unkana.pl
#
# (Version with multi-encoding support without using JPerl)
# Written by Ken Lunde (lunde@ora.com or lunde@adobe.com)
# August 13, 1996
#
# This program converts half-width katakana to their full-width
# counterparts in either Shift-JIS or EUC encoding (if the file is in
# EUC encoding, you must use the "-e" switch on the command line).
#
# Input is expected from stdin, and output is written to stdout.

require 5;

$counter = 0;
$euc = "";

if (defined $ARGV[0] && $ARGV[0] eq "-e") {
  $euc = chr(142);
}

if ($euc) { # If EUC encoding
  $encoding = '[\xA1-\xFE]{2}';
  $symbol_one = chr(161);
  $kana_one = chr(165);

  # Second-byte values (decimal) in EUC
  @two = (161, 163, 214, 215, 162, 166, 242, 161, 163, 165, 167, 169, 227,
    229, 231, 195, 188, 162, 164, 166, 168, 170, 171, 173, 175, 177, 179,
    181, 183, 185, 187, 189, 191, 193, 196, 198, 200, 202 .. 207, 210, 213,
    216, 219, 222 .. 226, 228, 230, 232 .. 237, 239, 243, 171, 172 );
} else { # If Shift-JIS encoding
  $encoding = '[\x81-\x9F\xE0-\xFC][\x40-\x7E\x80-\xFC]';
  $symbol_one = chr(129);
  $kana_one = chr(131);

  # Second-byte values (decimal) in Shift-JIS
  @two = (64, 66, 117, 118, 65, 69, 146, 64, 66, 68, 70, 72, 131, 133, 135, 98,
    91, 65, 67, 69, 71, 73, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94, 96, 99,
    101, 103, 105 .. 110, 113, 116, 119, 122, 125, 126, 128 .. 130, 132, 134,
    136 .. 141, 143, 147, 74, 75 );
}

# Initialize lookup structure for kana substitution (stored in %char_hash)
foreach $value (160 .. 223) {
  $char_hash{chr($value)} = chr($two[$counter]);
  $counter++;
}

# main loop
while (<STDIN>) {
  s/((?:[\x00-\x80])+ |     # ASCII or JIS-Roman
     (?:$encoding)+ |       # JIS X 0208-1990
     (?:${euc}[\xA0-\xDF])+ # Half-width katakana
    )/&dostuff($1)/egox;
  print STDOUT;
}

sub dostuff {
  my ($str) = @_;
  if ($str =~ /^${euc}[\xA0-\xDF]/o) {                   # If half-width kana
    $str =~ s/((?:$euc\xB3$euc\xDE)+ |                   # u + dakuten
               (?:${euc}[\xCA-\xCE]$euc\xDF)+ |          # KSTH-row + dakuten
               (?:${euc}[\xB6-\xC4\xCA-\xCE]$euc\xDE)+ | # H-row + handakuten
               (?:${euc}[\xA0-\xDF])                     # All other cases
              )/&han2zen($1)/egox;
  }
  return $str; # Returns ASCII/JIS-Roman and JIS X 0208-1990 as-is
}

sub han2zen {
  my ($hkana) = @_;

  if ($hkana =~ /^$euc\xB3$euc\xDE/o) { # Handle special "u + dakuten" case
    if ($euc) {
      $hkana =~ s/$euc\xB3$euc\xDE/\xA5\xF4/go;
    } else {
      $hkana =~ s/\xB3\xDE/\x83\x94/go;
    }
  } elsif ($hkana =~ /^${euc}[\xB6-\xC4\xCA-\xCE]${euc}[\xDE\xDF]/o) {
    $prefix = $kana_one;                   # First byte for katakana
    if ($hkana =~ /^${euc}[\xCA-\xCE]$euc\xDF/o) {
      $suffix = 2;                         # Increment value for handakuten
    } else {
      $suffix = 1;                         # Increment value for dakuten
    }
    $hkana =~ s/$euc([\xB6-\xC4\xCA-\xCE])${euc}[\xDE\xDF]/
                pack("n",unpack("n","$prefix$char_hash{$1}") + $suffix)/egox;
  } else {
    if ($hkana =~ /^${euc}[\xA0-\xA5\xB0\xDE\xDF]/o) {
      $prefix = $symbol_one;               # First byte for symbol
    } else {
      $prefix = $kana_one;                 # First byte for katakana
    }
    $hkana =~ s/$euc([\xA0-\xDF])/$prefix$char_hash{$1}/go;
  }
  return $hkana;
}
