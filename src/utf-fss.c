
/*
 * The following was provided by Ken Thompson of AT&T Bell Laboratories,
 * <ken@research.att.com>, on Tue, 8 Sep 92 03:22:07 EDT, to the X/Open
 * Joint Internationalization Group.  Some minor formatting changes have
 * been made by Glenn Adams, <glenn@metis.com>.
 *
 * -------------------------------------------------------------------------
 *  File System Safe Universal Character Set Transformation Format (FSS-UTF)
 * -------------------------------------------------------------------------
 *
 * With the approval of ISO/IEC 10646 (Unicode) as an international
 * standard and the anticipated wide spread use of this universal coded
 * character set (UCS), it is necessary for historically ASCII based
 * operating systems to devise ways to cope with representation and
 * handling of the large number of characters that are possible to be
 * encoded by this new standard.
 *
 * There are several challenges presented by UCS which must be dealt with
 * by historical operating systems and the C-language programming
 * environment.  The most significant of these challenges is the encoding
 * scheme used by UCS. More precisely, the challenge is the marrying of
 * the UCS standard with existing programming languages and existing
 * operating systems and utilities.
 *
 * The challenges of the programming languages and the UCS standard are
 * being dealt with by other activities in the industry.  However, we are
 * still faced with the handling of UCS by historical operating systems
 * and utilities.  Prominent among the operating system UCS handling
 * concerns is the representation of the data within the file system.  An
 * underlying assumption is that there is an absolute requirement to
 * maintain the existing operating system software investment while at
 * the same time taking advantage of the use the large number of
 * characters provided by the UCS.
 *
 * UCS provides the capability to encode multi-lingual text within a
 * single coded character set.  However, UCS and its UTF variant do not
 * protect null bytes and/or the ASCII slash ("/") making these character
 * encodings incompatible with existing Unix implementations.  The
 * following proposal provides a Unix compatible transformation format of
 * UCS such that Unix systems can support multi-lingual text in a single
 * encoding.  This transformation format encoding is intended to be used
 * as a file code.  This transformation format encoding of UCS is
 * intended as an intermediate step towards full UCS support.  However,
 * since nearly all Unix implementations face the same obstacles in
 * supporting UCS, this proposal is intended to provide a common and
 * compatible encoding during this transition stage.
 *
 * Goal/Objective
 * --------------
 *
 * With the assumption that most, if not all, of the issues surrounding
 * the handling and storing of UCS in historical operating system file
 * systems are understood, the objective is to define a UCS
 * transformation format which also meets the requirement of being usable
 * on a historical operating system file system in a non-disruptive
 * manner.  The intent is that UCS will be the process code for the
 * transformation format, which is usable as a file code.
 *
 * Criteria for the Transformation Format
 * --------------------------------------
 *
 * Below are the guidelines that were used in defining the UCS
 * transformation format:
 *
 *	1) Compatibility with historical file systems:
 *
 *	Historical file systems disallow the null byte and the ASCII
 *	slash character as a part of the file name.
 *
 *	2) Compatibility with existing programs:
 *
 *	The existing model for multibyte processing is that ASCII does
 *	not occur anywhere in a multibyte encoding.  There should be
 *	no ASCII code values for any part of a transformation format
 *	representation of a character that was not in the ASCII
 *	character set in the UCS representation of the character.
 *
 *	3) Ease of conversion from/to UCS.
 *
 *	4) The first byte should indicate the number of bytes to
 *	follow in a multibyte sequence.
 *
 *	5) The transformation format should not be extravagant in
 *	terms of number of bytes used for encoding.
 *
 *	6) It should be possible to find the start of a character
 *	efficiently starting from an arbitrary location in a byte
 *	stream.
 *
 * Proposed FSS-UTF
 * ----------------
 *
 * The proposed UCS transformation format encodes UCS values in the range
 * [0,0x7fffffff] using multibyte characters of lengths 1, 2, 3, 4, 5,
 * and 6 bytes.  For all encodings of more than one byte, the initial
 * byte determines the number of bytes used and the high-order bit in
 * each byte is set.  Every byte that does not start 10xxxxxx is the
 * start of a UCS character sequence.
 *
 * An easy way to remember this transformation format is to note that the
 * number of high-order 1's in the first byte signifies the number of
 * bytes in the multibyte character:
 *
 * Bits  Hex Min  Hex Max  Byte Sequence in Binary
 *   7  00000000 0000007f 0vvvvvvv
 *  11  00000080 000007FF 110vvvvv 10vvvvvv
 *  16  00000800 0000FFFF 1110vvvv 10vvvvvv 10vvvvvv
 *  21  00010000 001FFFFF 11110vvv 10vvvvvv 10vvvvvv 10vvvvvv
 *  26  00200000 03FFFFFF 111110vv 10vvvvvv 10vvvvvv 10vvvvvv 10vvvvvv
 *  31  04000000 7FFFFFFF 1111110v 10vvvvvv 10vvvvvv 10vvvvvv 10vvvvvv 10vvvvvv
 *
 * The UCS value is just the concatenation of the v bits in the multibyte
 * encoding.  When there are multiple ways to encode a value, for example
 * UCS 0, only the shortest encoding is legal.
 *
 * Below are sample implementations of the C standard wctomb() and
 * mbtowc() functions which demonstrate the algorithms for converting
 * from UCS to the transformation format and converting from the
 * transformation format to UCS. The sample implementations include error
 * checks, some of which may not be necessary for conformance:
 *
 */

typedef struct
{
  int     cmask;
  int     cval;
  int     shift;
  long    lmask;
  long    lval;
} Tab;

static Tab tab[] =
{
  0x80,  0x00,   0*6,    0x7F,           0,              /* 1 byte sequence */
  0xE0,  0xC0,   1*6,    0x7FF,          0x80,           /* 2 byte sequence */
  0xF0,  0xE0,   2*6,    0xFFFF,         0x800,          /* 3 byte sequence */
  0xF8,  0xF0,   3*6,    0x1FFFFF,       0x10000,        /* 4 byte sequence */
  0xFC,  0xF8,   4*6,    0x3FFFFFF,      0x200000,       /* 5 byte sequence */
  0xFE,  0xFC,   5*6,    0x7FFFFFFF,     0x4000000,      /* 6 byte sequence */
  0,                                                     /* end of table    */
};

int
  mbtowc ( wchar_t *p, char *s, size_t n )
{
  long l;
  int c0, c, nc;
  Tab *t;
  
  if ( s == 0 )
    return 0;
  
  nc = 0;
  if ( n <= nc )
    return -1;
  c0 = *s & 0xff;
  l = c0;
  for ( t = tab; t->cmask; t++ ) {
    nc++;
    if ( ( c0 & t->cmask ) == t->cval ) {
      l &= t->lmask;
      if ( l < t->lval )
	return -1;
      *p = l;
      return nc;
    }
    if ( n <= nc )
      return -1;
    s++;
    c = ( *s ^ 0x80 ) & 0xFF;
    if ( c & 0xC0 )
      return -1;
    l = ( l << 6 ) | c;
  }
  return -1;
}

int
  wctomb ( char *s, wchar_t wc )
{
  long l;
  int c, nc;
  Tab *t;
  
  if (s == 0 )
    return 0;
  
  l = wc;
  nc = 0;
  for ( t=tab; t->cmask; t++ ) {
    nc++;
    if ( l <= t->lmask ) {
      c = t->shift;
      *s = t->cval | ( l >> c );
      while ( c > 0 ) {
	c -= 6;
	s++;
	*s = 0x80 | ( ( l >> c ) & 0x3F );
      }
      return nc;
    }
  }
  return -1;
}
