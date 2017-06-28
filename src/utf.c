
/*
 * Copyright (C) 1992, Metis Technology, Inc.
 *
 * @(#)utf.c	1.4	11/2/92
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the Metis Library General Public License as
 * published by Metis Technology, Inc.; either version 2, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Metis Library General Public License for more details.
 *
 * In order to receive a copy of the Metis Library General Public License,
 * write to Metis Technology, Inc., 358 Windsor St, Cambridge, MA 02141, USA.
 * This license may also be retrieved by anonymous FTP from the file
 * "pub/LibraryPublicLicense" on the host METIS.COM [140.186.33.40].
 *
 *
 * Universal Character Set (UCS) Transformation Format (UTF) Utilities
 *
 * This file contains four functions which support conversion to and from
 * DIS10646-1.2:1992(E) UCS-2 and UCS-4 character code representations
 * and the UTF multi-octet format.  The detailed algorithm is specified in
 * ANNEX F of the above standard.
 *
 * The four functions defined below are:
 *
 *   int UCS2ToUTF ( UTFCHAR *  utf_str,
 *		     UCS2CHAR *	ucs_str,
 *		     int	utf_str_size )
 *
 *   int UTFToUCS2 ( UCS2CHAR * ucs_str,
 *		     UTFCHAR *  utf_str,
 *		     int        ucs_str_size )
 *
 *   int UCS4ToUTF ( UTFCHAR *  utf_str,
 *		     UCS4CHAR *	ucs_str,
 *		     int	utf_str_size )
 *
 *   int UTFToUCS4 ( UCS4CHAR * ucs_str,
 *		     UTFCHAR *  utf_str,
 *		     int        ucs_str_size )
 *
 *
 * For each function, the first argument represents a target buffer whose
 * size is specified by the third argument.  This size is not in bytes
 * but in element units according to the element type; e.g., bytes for
 * UTFCHAR, shorts for UCS2CHAR, and longs for UCS4CHAR (these types are
 * defined below).
 *
 * The source string pointed at by the second argument of each function
 * is transformed and copied into the target buffer.  The number of elements
 * consumed from the source string is returned.  In the case of UTFToUCS2,
 * if an invalid UCS-2 code value would be produced from the UTF string
 * data, i.e., a UTF string encoding a value outside the UCS-2 encoding
 * space, then the transformation is terminated and a -1 value is returned
 * indicating an error.
 *
 * If the target buffer is consumed prior to exhausting the source string,
 * transformation will be terminated.  In every case, except the condition
 * specified above for UTFToUCS2, the target buffer is null-terminated
 * at the termination of transformation.  Therefore, the size of the target
 * buffer should be adequate to store the transformed data plus a null
 * terminator (of the appropriate width, depending on the target type).
 *
 * If the target buffer was not large enough to hold one target element,
 * then none of the source is consumed, and a null terminator is placed
 * at the beginning of the target buffer.  In this case, a 0 value is
 * returned indicating that none of the source was consumed.
 * 
 * Notes:
 *
 * 1. Performance might be marginally improved by unrolling a few loops.
 *
 * 2. Interoperability between the UTF produced by this code has not
 *    been tested against another UTF implementation.
 *
 * 3. Upper bounds on target buffer sizes are as follows:
 *   
 *      UCS2ToUTF	Length ( ucs_str ) * 3	+ 1
 *      UTFToUCS2	Length ( utf_str )	+ 1
 *
 *      UCS4ToUTF	Length ( ucs_str ) * 5  + 1
 *      UTFToUCS4	Length ( utf_str )	+ 1
 *
 *    Length(x) returns the number of elements (not bytes) in x.  The
 *    result is the number of elements that the target buffer should be
 *    able to hold.  Room for the null terminator is included in these bounds.
 *
 * 4. Please report any bugs or suggestions to Glenn Adams, <glenn@metis.com>.
 *
 */

typedef unsigned long	CARD32;
typedef unsigned short	CARD16;
typedef unsigned char	CARD8;

typedef CARD16		UCS2CHAR;
typedef CARD32		UCS4CHAR;
typedef CARD8		UTFCHAR;

#define	UCS_RANGE0	0x00000000
#define	UCS_RANGE1	0x000000A0
#define	UCS_RANGE2	0x00000100
#define	UCS_RANGE3	0x00004016
#define	UCS_RANGE4	0x00038E2E

#define	UTF_RANGE0	0x00
#define	UTF_RANGE1	0xA0
#define	UTF_RANGE2	0xA1
#define	UTF_RANGE3	0xF6
#define	UTF_RANGE4	0xFC

static UTFCHAR T[] = {
  0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
  0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
  0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
  0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40,
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
  0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
  0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
  0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60,
  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
  0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
  0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
  0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0xa0, 0xa1,
  0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
  0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1,
  0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
  0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1,
  0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
  0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1,
  0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
  0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1,
  0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
  0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1,
  0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
  0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, 0x00, 0x01,
  0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
  0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
  0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
  0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x7f,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
  0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
};

static UTFCHAR U[] = {
  0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5,
  0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd,
  0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5,
  0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd,
  0xde, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
  0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
  0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
  0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e,
  0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
  0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
  0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
  0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
  0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
  0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e,
  0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
  0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0xdf,
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
  0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
  0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
  0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65,
  0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d,
  0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75,
  0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d,
  0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85,
  0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d,
  0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95,
  0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d,
  0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5,
  0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad,
  0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5,
  0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd,
};

int
  UCS2ToUTF ( utf_str, ucs_str, utf_str_size )
UTFCHAR *  utf_str;
UCS2CHAR * ucs_str;
int	   utf_str_size;
{
  register UCS2CHAR *p;
  register UTFCHAR *sp, *esp;
  CARD32 ucs;

  if ( ! utf_str || ! ucs_str || ! utf_str_size )
    return 0;

  sp  = utf_str;
  esp = & sp [ utf_str_size - 1 ];
  for ( p = ucs_str; ucs = (CARD32) *p; p++ ) {
    if ( ucs < UCS_RANGE1 ) {
      if ( ( esp - sp ) < 1 )
	break;
      *sp++ = (UTFCHAR) ucs;
    } else if ( ucs < UCS_RANGE2 ) {
      if ( ( esp - sp ) < 2 )
	break;
      *sp++ = (UTFCHAR) 0xA0;
      *sp++ = (UTFCHAR) ucs;
    } else if ( ucs < UCS_RANGE3 ) {
      register CARD32 y = ucs - 0x100;
      register CARD32 d = 0xBE;
      register int i;
      if ( ( esp - sp ) < 2 )
	break;
      *sp++ = (UTFCHAR) ( 0xA1 + y / d );
      for ( i = 0; i < 1; i++ )
	*sp++ = T [ ( y / ( d /= 0xBE ) ) % 0xBE ];
    } else {
      register CARD32 y = ucs - 0x4016;
      register CARD32 d = 0xBE * 0xBE;
      register int i;
      if ( ( esp - sp ) < 3 )
	break;
      *sp++ = (UTFCHAR) ( 0xF6 + y / d );
      for ( i = 0; i < 2; i++ )
	*sp++ = T [ ( y / ( d /= 0xBE ) ) % 0xBE ];
    }
  }
  *sp++ = (UTFCHAR) 0x00;
  return (int) ( p - ucs_str );
}


int
  UTFToUCS2 ( ucs_str, utf_str, ucs_str_size )
UCS2CHAR * ucs_str;
UTFCHAR *  utf_str;
int	   ucs_str_size;
{
  register UCS2CHAR *p, *ep;
  register UTFCHAR *sp, utf;
  CARD32 ucs;

  if ( ! ucs_str || ! utf_str || ! ucs_str_size )
    return 0;

  p   = ucs_str;
  ep  = & p  [ ucs_str_size - 1 ];
  for ( sp = utf_str; utf = *sp; sp++ ) {
    if ( utf < UTF_RANGE1 )
      ucs = (CARD32) utf;
    else if ( utf < UTF_RANGE2 )
      ucs = (CARD32) *++sp;
    else if ( utf < UTF_RANGE3 )
      ucs = (CARD32) ( utf - 0xA1 ) * 0xBE + U [ *++sp ] + 0x100;
    else if ( utf < UTF_RANGE4 ) {
      register int i;
      ucs = (CARD32) ( utf - 0xF6 );
      for ( i = 0; i < 2; i++ )
	ucs = ucs * 0xBE + U [ *++sp ];
      ucs += 0x4016;
    } else {
      register int i;
      ucs = (CARD32) ( utf - 0xFC );
      for ( i = 0; i < 4; i++ )
	ucs = ucs * 0xBE + U [ *++sp ];
      ucs += 0x38E2E;
    }
    if ( ( ep - p ) < 1 )
      break;
    if ( ucs > 0xffff )
      return -1;
    *p++ = ucs;
  }
  *p++ = (UCS2CHAR) 0x0000;
  return (int) ( sp - utf_str );
}

int
  UCS4ToUTF ( utf_str, ucs_str, utf_str_size )
UTFCHAR *  utf_str;
UCS4CHAR * ucs_str;
int	   utf_str_size;
{
  register UCS4CHAR *p;
  register UTFCHAR *sp, *esp;
  CARD32 ucs;

  if ( ! utf_str || ! ucs_str || ! utf_str_size )
    return 0;

  sp  = utf_str;
  esp = & sp [ utf_str_size - 1 ];
  for ( p = ucs_str; ucs = (CARD32) *p; p++ ) {
    if ( ucs < UCS_RANGE1 ) {
      if ( ( esp - sp ) < 1 )
	break;
      *sp++ = (UTFCHAR) ucs;
    } else if ( ucs < UCS_RANGE2 ) {
      if ( ( esp - sp ) < 2 )
	break;
      *sp++ = (UTFCHAR) 0xA0;
      *sp++ = (UTFCHAR) ucs;
    } else if ( ucs < UCS_RANGE3 ) {
      register CARD32 y = ucs - 0x100;
      register CARD32 d = 0xBE;
      register int i;
      if ( ( esp - sp ) < 2 )
	break;
      *sp++ = (UTFCHAR) ( 0xA1 + y / d );
      for ( i = 0; i < 1; i++ )
	*sp++ = T [ ( y / ( d /= 0xBE ) ) % 0xBE ];
    } else if ( ucs < UCS_RANGE4 ) {
      register CARD32 y = ucs - 0x4016;
      register CARD32 d = 0xBE * 0xBE;
      register int i;
      if ( ( esp - sp ) < 3 )
	break;
      *sp++ = (UTFCHAR) ( 0xF6 + y / d );
      for ( i = 0; i < 2; i++ )
	*sp++ = T [ ( y / ( d /= 0xBE ) ) % 0xBE ];
    } else {
      register CARD32 y = ucs - 0x38E2E;
      register CARD32 d = 0xBE * 0xBE * 0xBE * 0xBE;
      register int i;
      if ( ( esp - sp ) < 5 )
	break;
      *sp++ = (UTFCHAR) ( 0xFC + y / d );
      for ( i = 0; i < 4; i++ )
	*sp++ = T [ ( y / ( d /= 0xBE ) ) % 0xBE ];
    }
  }
  *sp++ = (UTFCHAR) 0x00;
  return (int) ( p - ucs_str );
}


int
  UTFToUCS4 ( ucs_str, utf_str, ucs_str_size )
UCS4CHAR * ucs_str;
UTFCHAR *  utf_str;
int	   ucs_str_size;
{
  register UCS4CHAR *p, *ep;
  register UTFCHAR *sp, utf;
  CARD32 ucs;

  if ( ! ucs_str || ! utf_str || ! ucs_str_size )
    return 0;

  p   = ucs_str;
  ep  = & p  [ ucs_str_size - 1 ];
  for ( sp = utf_str; utf = *sp; sp++ ) {
    if ( utf < UTF_RANGE1 )
      ucs = (CARD32) utf;
    else if ( utf < UTF_RANGE2 )
      ucs = (CARD32) *++sp;
    else if ( utf < UTF_RANGE3 )
      ucs = (CARD32) ( utf - 0xA1 ) * 0xBE + U [ *++sp ] + 0x100;
    else if ( utf < UTF_RANGE4 ) {
      register int i;
      ucs = (CARD32) ( utf - 0xF6 );
      for ( i = 0; i < 2; i++ )
	ucs = ucs * 0xBE + U [ *++sp ];
      ucs += 0x4016;
    } else {
      register int i;
      ucs = (CARD32) ( utf - 0xFC );
      for ( i = 0; i < 4; i++ )
	ucs = ucs * 0xBE + U [ *++sp ];
      ucs += 0x38E2E;
    }
    if ( ( ep - p ) < 1 )
      break;
    *p++ = ucs;
  }
  *p++ = (UCS4CHAR) 0x0000;
  return (int) ( sp - utf_str );
}
