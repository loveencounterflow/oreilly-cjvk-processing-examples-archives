/*
	Program:	WinJConv,Japanse Character set Convert DLL for Windows NT/95.
	Version:	0.80.
	Date:		July 19, 1996
	Ported by:	Norihito Nagase, CAST corporation.
	EMAIL:		nagase@cast.co.jp
	MAIL:		3-26-6 nishi-asakusa,taitou-ku,toukyou-to 111. JAPAN

1. INTRODUCTION:	
  This program is ported from famous Ken R. Lunde's JCONV.
  Freely distributable version. Please use your own risk!
  But, same as original JCONV, if this program may be bundled with
  commercial software, please contact us.(CAST Corp.)

-----!!!! IMPORTANT  This is original author's notes. IMPORTNAT !!!!-----
  Program: jconv.c
  Version: 3.0
  Date:    July 1, 1993
  Author:  Ken R. Lunde, Adobe Systems Incorporated
    EMAIL: lunde@mv.us.adobe.com
    MAIL : 1585 Charleston Road, P.O. Box 7900, Mountain View, CA 94039-7900
  Type:    A tool for converting the Japanese code of Japanese textfiles.
  Code:    ANSI C (portable)

2. DISTRIBUTION AND RESTRICTIONS ON USAGE:
   1) Please give this source code away to your friends at no charge.
   3) Only use this tool on a copy of a file -- do not use an original. This
      is just common sense.
   4) This source code or a compiled version may be bundled with commercial
      software as long as the author is notified beforehand. The author's name
     should also be mentioned in the credits.
   5) Feel free to use any of the algorithms for your own work. Many of them are
      being used in other tools I have written.

----!!!! End of original author's notes. !!!!----

  if you need more information, see his home page:
  			http://jasper.ora.com/lunde/
  	or his book:
    		Understanding Japanese Information Processing
  			ISBN 1-56592-043-0	
			O'REILLY. 
  Many thanks for Mr. Lunde !

3. DESCRIPTION:
   1) Supports Shift-JIS, EUC, New-JIS, Old-JIS, and NEC-JIS for both input and
      output.
   2) Automatically detects infile's Japanese code (the ability to force an
      input Japanese code is also supported).
   3) The ability to convert Shift-JIS and EUC half-width katakana into full-
      width equivalents. Note that half-width katakana includes other symbols
      such as a Japanese period, Japanese comma, center dot, etc.
   4) Supports conversion between the same code (i.e., EUC -> EUC, Shift-JIS ->
      Shift-JIS, etc.). This is useful as a filter for converting half-width
      katakana to their full-width equivalents.
   5) If the infile does not contain any Japanese, then its contents are
      echoed to the buffer.

4.ABOUT THIS VERSION(Ver. 0.80):
    Currently, tested on next function
    1) simple convert and convert with Hankaku kana to Full kana
  	  	NEW-JIS	->  S-JIS
		EUC	    ->  S-JIS
		S-JIS	->  S-JIS
    2) convert with repairs broken Jis in process
		not tested
    3) Escape Delete/Force Escape Delete
		not tested
    4) Auto-detects infile's Japanese code								  
	  	NEW-JIS,EUC,S-JIS
    If you test on other situation, please contact me.
     (Perhaps, original JCONV is all done, but I don't know it. Sorry!)

5.COMPILE ENVIRONMENTS.
  Compiled on MicroSoft VC++ 4.0 japanese edition.
  Please build your project with WinJConv.c and WinJConv.def.
  Since original code is ANSI-C standard, you don't need any MFC and other 
  librarys unless standard Windows 32 bit DLL modules.

6.How to get newest version of WinJConv.
  Please refer to following URL:
	ftp://ftp.cast.co.jp/pub/tools/japanese/
				->Our FTP site.
	http://www.cast.co.jp
				->Our web site.(but currentry under construction, sorry!)
	ftp://ftp.ora.com/pub/examples/nutshell/ujip/windows/ 
				-> O'REILLY's FTP site. Mirrored by Mr. Lunde.

*/
#include <windows.h>
//#include "stdafx.h"

#include <stdio.h>
//#include <string.h>
//#include <ctype.h>
//#include <errno.h>

#define INPUT       1
#define OUTPUT      2
#define REPAIR      3
#define NOTSET      0
//Here recognizeable character sets
#define NEW         1
#define OLD         2
#define NEC         3
#define EUC         4
#define SJIS        5
#define EUCORSJIS   6
#define ASCII       7

#define NUL         0
#define NL          10
#define CR          13
#define ESC         27
#define SS2         142
#define TRUE        1
#define FALSE       0
#define PERIOD      '.'
#define SJIS1(A)    ((A >= 129 && A <= 159) || (A >= 224 && A <= 239))
#define SJIS2(A)    (A >= 64 && A <= 252)
#define HANKATA(A)  (A >= 161 && A <= 223)
#define ISEUC(A)    (A >= 161 && A <= 254)
#define ISMARU(A)   (A >= 202 && A <= 206)
#define ISNIGORI(A) ((A >= 182 && A <= 196) || (A >= 202 && A <= 206) || (A == 179))

/* The following 5 lines of code are used to establish the default output codes
 *  They are self-explanatory, and easy to change.
 */
#define DEFAULT_O   SJIS     /* default output code */
#define DEFAULT_OKI ""       /* default kanji-in code */
#define DEFAULT_OKO ""       /* default kanji-out code */

#define FUNC_CONVERT		0
#define FUNC_CONVERTKANA	1
#define FUNC_REPAIR			2
#define FUNC_DELETEESC		3
#define FUNC_DELETEESCFORCE	4
#define FUNC_CHECKONLY		5
#define FUNC_DEBUG			9

void han2zen(FILE *in,int *p1,int *p2,int incode);
void sjis2jis(int *p1,int *p2);
void jis2sjis(int *p1,int *p2);
void shift2seven(FILE *in,LPSTR out,int incode,char ki[],char ko[]);
void shift2euc(FILE *in,LPSTR out,int incode,int tofullsize);
void euc2seven(FILE *in,LPSTR out,int incode,char ki[],char ko[]);
void euc2euc(FILE *in,LPSTR out,int incode,int tofullsize);
void shift2shift(FILE *in,LPSTR out,int incode,int tofullsize);
void euc2shift(FILE *in,LPSTR out,int incode,int tofullsize);
void seven2shift(FILE *in,LPSTR out);
void seven2euc(FILE *in,LPSTR out);
void seven2seven(FILE *in,LPSTR out,char ki[],char ko[]);
void asc2asc(FILE *in,LPSTR out);
void jisrepair(FILE *in,LPSTR out,int outcode,char ki[],char ko[]);
void removeescape(FILE *in,LPSTR out,int forcedelesc);
int SkipESCSeq(FILE *in,int temp,int *intwobyte);
int DetectCodeType(FILE *in);
void getcode(int data,char ki[],char ko[]);
void PutByte(LPSTR *out, int code);
void PutString(LPSTR *out, LPSTR kiko);
void PutDebugStr(LPSTR *out, LPSTR debugstr);

int	debugmode;

//					    
//	DLL Main 
//
BOOL WINAPI DllMain (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
  switch (dwReason)
  {
    case DLL_PROCESS_ATTACH:
      // DLL is attaching to the address space of the current process.
      break;
    case DLL_THREAD_ATTACH:
      // A new thread is being created in the current process.
       break;
    case DLL_THREAD_DETACH:
      // A thread is exiting cleanly.
      break;
    case DLL_PROCESS_DETACH:
      // The calling process is detaching the DLL from its address space.
      break;
  }

return TRUE;
}

/*******************************************************************************
*
*	FUNCTION:	WinJConv,Main body
*
*	IN:		
*		infilename:	input filename 
*		outbuf:		output characters, must NULL terminated!
*					(but not all, because this program ends when detect NULL in this buffer)
*		incode:		input character type(see below)
*		outcode:	output character type(see below)
*		  in/outcode charactercode codes 
*			0		in=auto detect, out=default charater set=S-JIS
*			1		NEW-JIS
*			2		OLD-JIS
*			3		NEC
*			4		EUC
*			5		S-JIS
*			6		EUCORSJIS
*			7		ASCII
*		function code:
*			0		simple convert
*			1		convert with Hankaku kana to Full Kana
*					valid when both input/output code are EUC/S-JIS
*			2		convert with repairs broken JIS in process
*			3		Escape Delete
*			4		Force Escape Delete
*			-> when 3 or 4 is set, input codes assumes New/Old JIS
*			5		Check only,determins input file character codes.
*			9		simple convert with debug string output,out code =S-JIS
*	OUT:
*		outbuf:		Convert result.
*	RETURNS:
*			-1=Input parameter missing
*			-2=Input file nothing, or input character code not detected
*			-3=Ambiguous (Shift-JIS or EUC) input code
*			OTHER=Input Character set Code
*
*******************************************************************************/

int WinJConv(LPSTR infilename, LPSTR out, int funccode, int incode, int outcode)
{
	int forcedelesc = FALSE, delesc = FALSE, checkonly = FALSE;
	int repairjis = FALSE, tofullsize = FALSE, docheck = FALSE;
	char ki[10],ko[10]; //, deb[20];
	FILE *in;
																				
	debugmode = FALSE;
	switch ( funccode ) {
    case FUNC_CONVERT :
		break;
	case FUNC_CONVERTKANA :
		tofullsize = TRUE;
		break;
	case FUNC_REPAIR :
		repairjis = TRUE;
		break;
	case FUNC_DELETEESC :
		delesc = TRUE;
		break;
	case FUNC_DELETEESCFORCE :
		delesc = TRUE;
		forcedelesc = TRUE;
		break;
	case FUNC_CHECKONLY :
		checkonly= TRUE;
		incode = 0;
		break;
	case FUNC_DEBUG	:
		debugmode = TRUE;
		outcode = 0;
		tofullsize = TRUE;
		PutDebugStr(&out,"WinJConv, Debug mode start ");
		break;
	default:
//		PutString(&out,"Invali function code=");
//		itoa(funccode, &out, 10);
//		PutString(&out,deb);
		return -1;
	}	

	if ( incode == NOTSET )    
		docheck = TRUE;
	if ( repairjis && delesc )
		return -1;
	if ( outcode == NOTSET && !delesc )
		outcode = DEFAULT_O;
    getcode(outcode,ki,ko);		//set Kanji In Kanji Out

	if ( (in = fopen(infilename,"r") ) == NULL) {
		return -2;
	}
	PutDebugStr(&out,"fopen done ");
	if ( repairjis ) {			//repair only
		jisrepair(in,out,outcode,ki,ko);
		fclose(in);
		return incode;
	}
	if ( delesc ) {				//remove escape code
		removeescape(in,out,forcedelesc);
		fclose(in);
		return incode;
	}
	if ( docheck ) {
	    incode = DetectCodeType(in);
		PutDebugStr(&out,"DetectCodeType done ");
		if ( checkonly ) {
			fclose(in);
			return incode;
		}
		rewind(in);
	}
	switch ( incode ) {
    case NOTSET :
		fclose(in);
		return -2;
		break;
    case EUCORSJIS :
		fclose(in);
		return -3;
		break;
    case ASCII :
		asc2asc(in,out);		//simple copy
		return incode;			//nothing convert
		break;
    case NEW :
    case OLD :
    case NEC :
		switch ( outcode ) {
        case NEW :
        case OLD :
        case NEC :
			seven2seven(in,out,ki,ko);
			break;
        case EUC :
			seven2euc(in,out);
			break;
        case SJIS :
			seven2shift(in,out);
			break;
		}
		break;
    case EUC :
		switch ( outcode ) {
        case NEW :
        case OLD :
        case NEC :
			euc2seven(in,out,incode,ki,ko);
			break;
        case EUC :
			euc2euc(in,out,incode,tofullsize);
			break;
        case SJIS :
			euc2shift(in,out,incode,tofullsize);
			break;
		}
		break;
    case SJIS :
		switch ( outcode ) {
        case NEW :
        case OLD :
        case NEC :
			shift2seven(in,out,incode,ki,ko);
			break;
        case EUC :
			shift2euc(in,out,incode,tofullsize);
			break;
        case SJIS :
			shift2shift(in,out,incode,tofullsize);
			break;
		}
		break;
	default :					//invalid input character code
		return -1;
		break;
	}
	fclose(in);
	return incode;
}

void getcode(int data,char ki[],char ko[])
{
	if ( data == NEW ) {
		strcpy(ki,"$B");
		strcpy(ko,"(J");
	} else if ( data == OLD ) {
		strcpy(ki,"$@");
		strcpy(ko,"(J");
	} else if ( data == NEC ) {
		strcpy(ki,"K");
		strcpy(ko,"H");
	} else {
		strcpy(ki,DEFAULT_OKI);
		strcpy(ko,DEFAULT_OKO);
	}
}

int SkipESCSeq(FILE * in,int temp,int *intwobyte)
{
  int tempdata = *intwobyte;

	if ( temp == '$' || temp == '(' )
		fgetc(in);
	if ( temp == 'K' || temp == '$' )
		*intwobyte = TRUE;
	else
		*intwobyte = FALSE;
	if ( tempdata == *intwobyte )
		return FALSE;
	else
		return TRUE;
}
 
void removeescape(FILE *in, LPSTR out,int forcedelesc)
{
	int p1,p2,p3;
	unsigned long count = 0,other = 0;

	PutDebugStr(&out,"removeescapestart ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( p1 == ESC ) {
			p2 = fgetc(in);
			if ( p2 == '(' ) {
				p3 = fgetc(in);
				switch (p3) {
				case 'J' :
				case 'B' :
				case 'H' :
					PutByte(&out,p2);
					PutByte(&out,p3);
					count++;
					break;
				default :
					if (forcedelesc) {
						PutByte(&out,p2);
						PutByte(&out,p3);
					} else {
						PutByte(&out,p1);
						PutByte(&out,p2);
						PutByte(&out,p3);
						other++;
					}
					break;
				}
			} else if ( p2 == '$' ) {
				p3 = fgetc(in);
				switch (p3) {
				case 'B' :
				case '@' :
					PutByte(&out,p2);
					PutByte(&out,p3);
					count++;
					break;
				default :
					if ( forcedelesc ) {
						PutByte(&out,p2);
						PutByte(&out,p3);
					} else {
						PutByte(&out,p1);
						PutByte(&out,p2);
						PutByte(&out,p3);
					}
					other++;
					break;
				}
			} else {
				if ( forcedelesc )
					PutByte(&out,p2);
				else {
					PutByte(&out,p1);
					PutByte(&out,p2);
				}
				other++;
			}
		} else
			PutByte(&out,p1);
	}
}

void jisrepair(FILE *in,LPSTR out,int outcode,char ki[],char ko[])
{
	int p1,p2,p3,intwobyte = FALSE;
	unsigned long count = 0;
	
	PutDebugStr(&out,"jisrepair start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( intwobyte ) {
			if ( p1 == ESC ) {
				p2 = fgetc(in);
				if ( p2 == '(' ) {
					p3 = fgetc(in);
					switch (p3) {
					case 'J' :
					case 'B' :
					case 'H' :
						intwobyte = FALSE;
						switch (outcode) {
						case NEC :
						case NEW :
						case OLD :
							PutByte(&out,ESC);
							PutString(&out,ko);
							break;
						default :
							break;
						}
						break;
					default :
						PutByte(&out,p1);
						PutByte(&out,p2);
						PutByte(&out,p3);
						break;
					}
				} else if ( p2 == 'H' ) {
					intwobyte = FALSE;
					switch (outcode) {
					case NEC :
					case NEW :
					case OLD :
						PutByte(&out,ESC);
						PutString(&out,ko);
						break;
				    default :
						break;
					}
				} else
					PutByte(&out,p1);
					PutByte(&out,p2);
			} else if ( p1 == '(' ) {
				p2 = fgetc(in);
				switch (p2) {
				case 'J' :
				case 'B' :
				case 'H' :
					intwobyte = FALSE;
					switch (outcode) {
					case NEC :
					case NEW :
					case OLD :
						PutByte(&out,ESC);
						PutString(&out,ko);
		                break;
					default :
					    break;
					}
					count++;
					break;
				default :
					switch (outcode) {
					case NEC :
					case NEW :
					case OLD :
		                PutByte(&out,p1);
						PutByte(&out,p2);
		                break;
					case EUC :
					    p1 += 128;
						p2 += 128;
		                PutByte(&out,p1);
						PutByte(&out,p2);
						break;
					case SJIS :
						jis2sjis(&p1,&p2);
	               		PutByte(&out,p1);
						PutByte(&out,p2);
						break;
					}
					break;
				}
			} else if ( p1 == NL ) {
				switch (outcode) {
				case NEC :
				case NEW :
				case OLD :
					PutByte(&out,ESC);
					PutString(&out,ko);
					PutByte(&out,p1);
					break;
				default :
					PutByte(&out,p1);
					break;
				}
				count++;
				intwobyte = FALSE;
			} else {
				p2 = fgetc(in);
				switch (outcode) {
				case NEC :
				case NEW :
				case OLD :
					PutByte(&out,p1);
					PutByte(&out,p2);
					break;
				case EUC :
					p1 += 128;
					p2 += 128;
					PutByte(&out,p1);
					PutByte(&out,p2);
					break;
				case SJIS :
					jis2sjis(&p1,&p2);
					PutByte(&out,p1);
					PutByte(&out,p2);
					break;
				}
			}
		} else {
			if ( p1 == ESC ) {
				p2 = fgetc(in);
				if ( p2 == '$' ) {
					p3 = fgetc(in);
					switch (p3) {
					case 'B' :
					case '@' :
						intwobyte = TRUE;
						switch (outcode) {
						case NEC :
						case NEW :
						case OLD :
	                  		PutByte(&out,ESC);
							PutString(&out,ki);
							break;
						default :
							break;
						}
						break;
					default :
						PutByte(&out,p1);
						PutByte(&out,p2);
						PutByte(&out,p3);
						break;
					}
				} else if ( p2 == 'K' ) {
					intwobyte = TRUE;
					switch (outcode) {
					case NEC :
					case NEW :
					case OLD :
	                  	PutByte(&out,ESC);
						PutString(&out,ki);
						break;
					default :
						break;
					}
				} else {
					PutByte(&out,p1);
					PutByte(&out,p2);
				}
			} else if ( p1 == '$' ) {
				p2 = fgetc(in);
				switch (p2) {
				case 'B' :
				case '@' :
					intwobyte = TRUE;
					switch (outcode) {
					case NEC :
					case NEW :
					case OLD :
		                PutByte(&out,ESC);
						PutString(&out,ki);
						break;
					default :
						break;
					}
					count++;
					break;
				default :
					switch (outcode) {
					case NEC :
					case NEW :
					case OLD :
						PutByte(&out,p1);
						PutByte(&out,p2);
						break;
					case EUC :
						PutByte(&out,p1);
						PutByte(&out,p2);
						break;
					case SJIS :
						PutByte(&out,p1);
						PutByte(&out,p2);
						break;
					}
					break;
				}
			} else
				PutByte(&out,p1);
		}
	}
	if ( intwobyte ) {
		switch (outcode) {
		case NEC :
		case NEW :
		case OLD :
			PutByte(&out,ESC);
			PutString(&out,ko);
			count++;
			break;
		default :
			break;
		}
	}
}

void sjis2jis(int *p1, int *p2)
{
	unsigned char c1 = *p1;
	unsigned char c2 = *p2;
	int adjust = c2 < 159;
	int rowOffset = c1 < 160 ? 112 : 176;
	int cellOffset = adjust ? (c2 > 127 ? 32 : 31) : 126;

	*p1 = ((c1 - rowOffset) << 1) - adjust;
	*p2 -= cellOffset;
}

void jis2sjis(int *p1, int *p2)
{
	unsigned char c1 = *p1;
	unsigned char c2 = *p2;
	int rowOffset = c1 < 95 ? 112 : 176;
	int cellOffset = c1 % 2 ? (c2 > 95 ? 32 : 31) : 126;

	*p1 = ((c1 + 1) >> 1) + rowOffset;
	*p2 += cellOffset;
}

void shift2seven(FILE *in, LPSTR out, int incode, char ki[], char ko[])
{
	int p1,p2,intwobyte = FALSE;

	PutDebugStr(&out,"shift2seven start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( p1 == NL || p1 == CR ) {
			if ( intwobyte ) {
				intwobyte = FALSE;
				PutByte(&out,ESC);
				PutString(&out,ko);
			}
			PutByte(&out,p1);
		} else if ( SJIS1(p1) ) {
			p2 = fgetc(in);
			if ( SJIS2(p2) ) {
				sjis2jis(&p1,&p2);
				if ( !intwobyte ) {
					intwobyte = TRUE;
					PutByte(&out,ESC);
					PutString(&out,ki);
				}
			}
			PutByte(&out,p1);
			PutByte(&out,p2);
		} else if ( HANKATA(p1) ) {
			han2zen(in,&p1,&p2,incode);
			sjis2jis(&p1,&p2);
			if ( !intwobyte ) {
				intwobyte = TRUE;
				PutByte(&out,ESC);
				PutString(&out,ki);
			}
			PutByte(&out,p1);
			PutByte(&out,p2);
		} else {
			if ( intwobyte ) {
				intwobyte = FALSE;
				PutByte(&out,ESC);
				PutString(&out,ko);
			}
			PutByte(&out,p1);
		}
	}
	if ( intwobyte ) {
		PutByte(&out,ESC);
		PutString(&out,ko);
	}
}

void shift2euc(FILE *in, LPSTR out, int incode, int tofullsize)
{
	int p1,p2;
  
	PutDebugStr(&out,"shift2euc start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( SJIS1(p1) ) {
			p2 = fgetc(in);
			if ( SJIS2(p2) ) {
				sjis2jis(&p1,&p2);
				p1 += 128;
				p2 += 128;
			}
			PutByte(&out,p1);
			PutByte(&out,p2);
		} else if ( HANKATA(p1) ) {
			if ( tofullsize ) {
				han2zen(in,&p1,&p2,incode);
				sjis2jis(&p1,&p2);
				p1 += 128;
				p2 += 128;
			} else {
				p2 = p1;
				p1 = SS2;
			}
			PutByte(&out,p1);
			PutByte(&out,p2);
		} else
			PutByte(&out,p1);
	}
}
 
void euc2seven(FILE *in, LPSTR out, int incode, char ki[], char ko[])
{
	int p1,p2,intwobyte = FALSE;

	PutDebugStr(&out,"euc2seven start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( p1 == NL || p1 == CR ) {
			if ( intwobyte ) {
				intwobyte = FALSE;
				PutByte(&out,ESC);
				PutString(&out,ko);
			}
			PutByte(&out,p1);
		} else {
			if ( ISEUC(p1) ) {
				p2 = fgetc(in);
				if ( ISEUC(p2) ) {
					p1 -= 128;
					p2 -= 128;
					if ( !intwobyte ) {
						intwobyte = TRUE;
						PutByte(&out,ESC);
						PutString(&out,ki);
					}
				}
				PutByte(&out,p1);
				PutByte(&out,p2);
			} else if ( p1 == SS2 ) {
				p2 = fgetc(in);
				if ( HANKATA(p2) ) {
					p1 = p2;
					han2zen(in,&p1,&p2,incode);
					sjis2jis(&p1,&p2);
					if ( !intwobyte ) {
						intwobyte = TRUE;
						PutByte(&out,ESC);
						PutString(&out,ki);
					}
				}
				PutByte(&out,p1);
				PutByte(&out,p2);
			} else {
				if ( intwobyte ) {
					intwobyte = FALSE;
					PutByte(&out,ESC);
					PutString(&out,ko);
				}
				PutByte(&out,p1);
			}	
		}
	}
	if ( intwobyte ) {
		PutByte(&out,ESC);
		PutString(&out,ko);
	}
}
 
void euc2shift(FILE *in,LPSTR out,int incode,int tofullsize)
{
	int p1,p2;

	PutDebugStr(&out,"euc2shift  start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( ISEUC(p1) ) {
			p2 = fgetc(in);
			if ( ISEUC(p2) ) {
				p1 -= 128;
				p2 -= 128;
				jis2sjis(&p1,&p2);
			}
			PutByte(&out,p1);
			PutByte(&out,p2);
		} else if ( p1 == SS2 ) {
			p2 = fgetc(in);
			if ( HANKATA(p2) ) {
				p1 = p2;
				if ( tofullsize ) {
					han2zen(in,&p1,&p2,incode);
					PutByte(&out,p1);
					PutByte(&out,p2);
				} else {
					PutByte(&out,p1);
				}
			} else {
				PutByte(&out,p1);
				PutByte(&out,p2);
			}
		} else
			PutByte(&out,p1);
	}
}

void euc2euc(FILE *in, LPSTR out, int incode, int tofullsize)
{
	int p1,p2;

	PutDebugStr(&out,"euc2euc  start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( ISEUC(p1) ) {
			p2 = fgetc(in);
			if ( ISEUC(p2) ) {
				PutByte(&out,p1);
				PutByte(&out,p2);
			}
		} else if ( p1 == SS2 ) {
			p2 = fgetc(in);
			if ( HANKATA(p2) && tofullsize ) {
				p1 = p2;
				han2zen(in,&p1,&p2,incode);
				sjis2jis(&p1,&p2);
				p1 += 128;
				p2 += 128;
			}
			PutByte(&out,p1);
			PutByte(&out,p2);
		} else
			PutByte(&out,p1);
	}
}

void shift2shift(FILE *in, LPSTR out, int incode, int tofullsize)
{
	int p1,p2;
  
	PutDebugStr(&out,"shift2shift  start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( SJIS1(p1) ) {
			p2 = fgetc(in);
			if ( SJIS2(p2) ) {
				PutByte(&out,p1);
				PutByte(&out,p2);
			}
		} else if ( HANKATA(p1) && tofullsize ) {
			han2zen(in,&p1,&p2,incode);
			PutByte(&out,p1);
			PutByte(&out,p2);
		} else
			PutByte(&out,p1);
	}
}

void seven2shift(FILE *in, LPSTR out)
{
	int temp,p1,p2,intwobyte = FALSE;

	PutDebugStr(&out,"seven2shift start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( p1 == ESC ) {
			temp = fgetc(in);
			SkipESCSeq(in,temp,&intwobyte);
		} else if ( p1 == NL || p1 == CR ) {
			if ( intwobyte )
				intwobyte = FALSE;
			PutByte(&out,p1);
		} else {
			if ( intwobyte ) {
				p2 = fgetc(in);
				jis2sjis(&p1,&p2);
				PutByte(&out,p1);
				PutByte(&out,p2);
			} else
				PutByte(&out,p1);
		}
	}
}
  
void seven2euc(FILE *in, LPSTR out)
{
	int temp,p1,p2,intwobyte = FALSE;

	PutDebugStr(&out,"seven2euc start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( p1 == ESC ) {
			temp = fgetc(in);
			SkipESCSeq(in,temp,&intwobyte);
		} else if ( p1 == NL || p1 == CR ) {
			if ( intwobyte )
				intwobyte = FALSE;
			PutByte(&out,p1);
		} else {
			if ( intwobyte ) {
				p2 = fgetc(in);
				p1 += 128;
				p2 += 128;
				PutByte(&out,p1);
				PutByte(&out,p2);
			} else
				PutByte(&out,p1);
		}
	}
}

void seven2seven(FILE *in, LPSTR out, char ki[], char ko[])
{
	int temp,p1,p2,change,intwobyte = FALSE;

	PutDebugStr(&out,"seven2seven start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		if ( p1 == ESC ) {
			temp = fgetc(in);
			change = SkipESCSeq(in,temp,&intwobyte);
			if ( (intwobyte) && (change) ) {
				PutByte(&out,ESC);
				PutString(&out,ki);
			} else if ( change ) {
				PutByte(&out,ESC);
				PutString(&out,ko);
			}
		} else if ( p1 == NL || p1 == CR ) {
			if ( intwobyte ) {
				intwobyte = FALSE;
				PutByte(&out,ESC);
				PutString(&out,ko);
			}
			PutByte(&out,p1);
		} else {
			if ( intwobyte ) {
				p2 = fgetc(in);
				PutByte(&out,p1);
				PutByte(&out,p2);
			} else
				PutByte(&out,p1);
		}
	}
	if ( intwobyte ) {
		PutByte(&out,ESC);
		PutString(&out,ko);
	}
}

void asc2asc(FILE *in, LPSTR out)
{
	int p1;

	PutDebugStr(&out,"asc2asc start ");
	while ( (p1 = fgetc(in)) != EOF && *out ) {
		PutByte(&out,p1);
	}
}

int DetectCodeType(FILE *in)
{
	int c = 0,whatcode = ASCII;

	while ( (whatcode == EUCORSJIS || whatcode == ASCII) && c != EOF ) {
		if ( (c = fgetc(in)) != EOF ) {
			if ( c == ESC ) {
				c = fgetc(in);
				if ( c == '$' ) {
					c = fgetc(in);
					if ( c == 'B' )
						whatcode = NEW;
					else if ( c == '@' )
						whatcode = OLD;
				} else if ( c == 'K' )
					whatcode = NEC;
			} else if ( (c >= 129 && c <= 141) || (c >= 143 && c <= 159) ) {
				whatcode = SJIS;
			} else if ( c == SS2 ) {
				c = fgetc(in);
				if ( (c >= 64 && c <= 126) || (c >= 128 && c <= 160) || (c >= 224 && c <= 252) )
					whatcode = SJIS;
				else if ( c >= 161 && c <= 223 )
					whatcode = EUCORSJIS;
			} else if ( c >= 161 && c <= 223 ) {
				c = fgetc(in);
				if ( c >= 240 && c <= 254 )
					whatcode = EUC;
				else if ( c >= 161 && c <= 223 )
					whatcode = EUCORSJIS;
				else if ( c >= 224 && c <= 239 ) {
					whatcode = EUCORSJIS;
					while ( c >= 64 && c != EOF && whatcode == EUCORSJIS ) {
						if ( c >= 129 ) {
							if ( c <= 141 || (c >= 143 && c <= 159) )
								whatcode = SJIS;
							else if ( c >= 253 && c <= 254 )
								whatcode = EUC;
						}
						c = fgetc(in);
					}
				} else if ( c <= 159 )
					whatcode = SJIS;
			} else if ( c >= 240 && c <= 254 )
				whatcode = EUC;
			else if ( c >= 224 && c <= 239 ) {
				c = fgetc(in);
				if ( (c >= 64 && c <= 126) || (c >= 128 && c <= 160) )
					whatcode = SJIS;
				else if ( c >= 253 && c <= 254 )
					whatcode = EUC;
				else if ( c >= 161 && c <= 252 )
					whatcode = EUCORSJIS;
			}
		}
	}
	return whatcode;
}

void han2zen(FILE *in,int *p1,int *p2,int incode)
{
	int tmp = *p1,junk,maru = FALSE,nigori = FALSE;
	int mtable[][2] = {
	{129,66},{129,117},{129,118},{129,65},{129,69},{131,146},{131,64},
    {131,66},{131,68},{131,70},{131,72},{131,131},{131,133},{131,135},
    {131,98},{129,91},{131,65},{131,67},{131,69},{131,71},{131,73},
    {131,74},{131,76},{131,78},{131,80},{131,82},{131,84},{131,86},
    {131,88},{131,90},{131,92},{131,94},{131,96},{131,99},{131,101},
    {131,103},{131,105},{131,106},{131,107},{131,108},{131,109},
    {131,110},{131,113},{131,116},{131,119},{131,122},{131,125},
    {131,126},{131,128},{131,129},{131,130},{131,132},{131,134},
    {131,136},{131,137},{131,138},{131,139},{131,140},{131,141},
    {131,143},{131,147},{129,74},{129,75}
	};

	if ( incode == SJIS ) {
		*p2 = fgetc(in);
		if ( *p2 == 222 ) {
			if ( ISNIGORI(*p1) )
				nigori = TRUE;
			else
				ungetc( *p2,in );
		} else if ( *p2 == 223 ) {
			if ( ISMARU(*p1) )
				maru = TRUE;
			else
				ungetc(*p2,in);
		} else
			ungetc(*p2,in);
	} else if ( incode == EUC ) {
		junk = fgetc(in);
		if ( junk == SS2 ) {
			*p2 = fgetc(in);
			if ( *p2 == 222 ) {
				if ( ISNIGORI(*p1) )
					nigori = TRUE;
				else {
					ungetc(*p2,in);
					ungetc(junk,in);
				}
			} else if ( *p2 == 223 ) {
				if ( ISMARU(*p1) )
					maru = TRUE;
				else {
					ungetc(*p2,in);
					ungetc(junk,in);
				}
			} else {
				ungetc(*p2,in);
				ungetc(junk,in);	
			}
		} else
			ungetc(junk,in);
	}
	if ( *p1 >= 161 && *p1 <= 223 ) {
		*p1 = mtable[tmp - 161][0];
		*p2 = mtable[tmp - 161][1];
	}
	if ( nigori ) {
		if ( (*p2 >= 74 && *p2 <= 103) || (*p2 >= 110 && *p2 <= 122) )
			(*p2)++;
		else if ( *p1 == 131 && *p2 == 69 )
			*p2 = 148;
	} else if ( maru && *p2 >= 110 && *p2 <= 122 )
		*p2 += 2;
}

void PutByte(LPSTR *out, int code)
{
	if ( **out ) {
		**out = (CHAR)code;
		(*out)++;
	}
}

void PutString(LPSTR *out, LPSTR str)
{
	while ( **out && *str ) {
		**out = *str++;
		(*out)++;
	}
}

void PutDebugStr(LPSTR *out, LPSTR debugstr)
{
	if ( !debugmode )
		return;
	while ( **out && *debugstr ) {
		**out = *debugstr++;
		(*out)++;
	}
}
