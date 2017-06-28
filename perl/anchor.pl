#!/usr/local/bin/perl -w

# Multiple-byte anchoring when matching Shift-JIS-encoded text

$search = "\x8C\x95";                        # A
$text1  = "Text 1 \x90\x56\x8C\x95\x93\xB9"; # BAC
$text2  = "Text 2 \x94\x92\x8C\x8C\x95\x61"; # DEF
$encoding = qq< # Shift-JIS encoding
  [\x00-\x80\xFD-\xFF]|       # ASCII and other one-byte
  [\xA0-\xDF]|                # Half-width katakana
  [\x81-\x9F\xE0-\xFC][\x40-\x7E\x80-\xFC] # Two-byte range
>;

print "First attempt -- no anchoring\n";
print " Matched Text1\n" if $text1 =~ /$search/o;
print " Matched Text2\n" if $text2 =~ /$search/o;

print "Second attempt -- anchoring\n";
print " Matched Text1\n" if $text1 =~ /^(?:$encoding)*?$search/ox;
print " Matched Text2\n" if $text2 =~ /^(?:$encoding)*?$search/ox;
