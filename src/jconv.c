/*
Program: jconv.c
Version: 3.0
Date:    July 1, 1993
Author:  Ken R. Lunde, Adobe Systems Incorporated
  EMAIL: lunde@mv.us.adobe.com
  MAIL : 1585 Charleston Road, P.O. Box 7900, Mountain View, CA 94039-7900
Type:    A tool for converting the Japanese code of Japanese textfiles.
Code:    ANSI C (portable)

PORTABILITY:
This source code was written so that it would be portable on C compilers which
conform to the ANSI C standard. It has been tested on a variety of compilers.

I used THINK C and GNU C as my development platforms. I left in the Macintosh-
specific lines of code so that it would be easier to enhance/debug this tool
later. For those of you who wish to use this tool on the Macintosh, simply
add the ANSI library to the THINK C project, and then build the application.
Be sure that THINK_C has been defined, though, as the conditional compilation
depends on it. You then have a double-clickable application, which when
launched, will greet you with a Macintosh-style interface.

DISTRIBUTION AND RESTRICTIONS ON USAGE:
 1) Please give this source code away to your friends at no charge.
 2) Please try to compile this source code on various platforms to check for
    portablity, and please report back to me with any results be they good or
    bad. Suggestions are always welcome.
 3) Only use this tool on a copy of a file -- do not use an original. This
    is just common sense.
 4) This source code or a compiled version may be bundled with commercial
    software as long as the author is notified beforehand. The author's name
    should also be mentioned in the credits.
 5) Feel free to use any of the algorithms for your own work. Many of them are
    being used in other tools I have written.
 6) The most current version can be obtained by requesting a copy directly
    from me.

DESCRIPTION:
 1) Supports Shift-JIS, EUC, New-JIS, Old-JIS, and NEC-JIS for both input and
    output.
 2) Automatically detects infile's Japanese code (the ability to force an
    input Japanese code is also supported through a command-line option).
 3) The ability to convert Shift-JIS and EUC half-width katakana into full-
    width equivalents. Note that half-width katakana includes other symbols
    such as a Japanese period, Japanese comma, center dot, etc.
 4) Supports conversion between the same code (i.e., EUC -> EUC, Shift-JIS ->
    Shift-JIS, etc.). This is useful as a filter for converting half-width
    katakana to their full-width equivalents.
 5) If the infile does not contain any Japanese, then its contents are
    echoed to the outfile. It will also try to repair the file as though
    it had stripped escape characters (see #6 below).
 6) The functionality of my other tool called repair-jis.c is included in
    this tool by using the "-r[CODE]" option. This will recover stripped
    escape characters, then convert the file to be in CODE format.

FILE HANDLING:
 1) Specifying the infile is optional as one can redirect stdin and stdout.
 2) Specifying the outfile is also optional. If none is specified, the
    tool will semi-intelligently change the file's name. The tool simply
    simply scans the outname, finds the last period in it, terminates
    the string at that point, and tacks on one of seven possible extensions.
    Here are some example command lines, and the resulting outfile names:

    a) jconv -oe sig.jpn                  = sig.euc
    b) jconv sig.jpn                      = sig.sjs (defaulted to Shift-JIS)
    c) jconv -oj sig.jpn.txt              = sig.jpn.new
    d) jconv -oo sig                      = sig.old

    This is very useful for MS-DOS users since a filename such as sig.jpn.new
    will not result in converting a file called sig.jpn.

    Also note that if the outfile and infile have the same name, the tool
    will not work, and data will be lost. I tried to build safe-guards against
    this. For example, note how this tool will change the outfile name so
    that it does not overwrite the infile:
    
    a) jconv -f sig.sjs                   = sig-.sjs
    b) jconv -f sig.sjs sig.sjs           = sig-.sjs
    c) jconv sig-.sjs                     = sig--.sjs
    
    If only the infile is given, a hyphen is inserted after the last period,
    and the extension is then reattached. If the outfile is specified by the
    user, then it will search for the last period (if any), attach a hyphen,
    and finally attach the proper extension). This sort of protection is NOT
    available from this tool if stdin/stdout are used.
 3) If you want to specify an infile, but to have the output to go to stdout,
    you have two options:

    a) jconv < sig.jpn                    <= redirect stdin for infile
    b) jconv sig.jpn -                    <= specify infile, then use "-" to
                                             indictate stdout for output

 4) When using pipes under UNIX, it is very important to use the "-iCODE"
    option so that the automatic code detection routines are skipped. This
    is because the tool attempts to use the rewind() function, and this
    has no effect on some machines if input is coming from a pipe. One can
    easily use the "-c" option to find out what the infile code is, then
    apply it in a pipe. Here are some examples (they assume that the infile
    code is EUC):

    a) cat japan.txt | jconv -oj          <= does not work
    a) cat japan.txt | jconv -ie -oj      <= works
*/

#ifdef THINK_C
#include <console.h>
#include <stdlib.h>
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifndef THINK_C
#include <errno.h>
#endif

#define INPUT       1
#define OUTPUT      2
#define REPAIR      3
#define NOTSET      0
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
#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif
/* The following 8 lines of code are used to establish the default output codes
 * when using the "-o[CODE]" or "-r[CODE]" options. They are self-explanatory,
 * and easy to change.
 */
#define DEFAULT_O   SJIS     /* default output code for "-o[CODE]" option */
#define DEFAULT_OS  ".sjs"   /* default file extension for "-o[CODE]" option */
#define DEFAULT_OKI ""       /* default kanji-in code for "-o[CODE]" option */
#define DEFAULT_OKO ""       /* default kanji-out code for "-o[CODE]" option */
#define DEFAULT_R   NEW      /* default output code for "-r[CODE]" option */
#define DEFAULT_RS  ".new"   /* default file extension for "-r[CODE]" option */
#define DEFAULT_RKI "$B"     /* default kanji-in code for "-r[CODE]" option */
#define DEFAULT_RKO "(J"     /* default kanji-out code for "-r[CODE]" option */

void han2zen(FILE *in,int *p1,int *p2,int incode);
void sjis2jis(int *p1,int *p2);
void jis2sjis(int *p1,int *p2);
void shift2seven(FILE *in,FILE *out,int incode,char ki[],char ko[]);
void shift2euc(FILE *in,FILE *out,int incode,int tofullsize);
void euc2seven(FILE *in,FILE *out,int incode,char ki[],char ko[]);
void euc2euc(FILE *in,FILE *out,int incode,int tofullsize);
void shift2shift(FILE *in,FILE *out,int incode,int tofullsize);
void euc2shift(FILE *in,FILE *out,int incode,int tofullsize);
void seven2shift(FILE *in,FILE *out);
void seven2euc(FILE *in,FILE *out);
void seven2seven(FILE *in,FILE *out,char ki[],char ko[]);
void dohelp(char toolname[]);
void dojistable(void);
void doeuctable(void);
void dosjistable(void);
void jisrepair(FILE *in,FILE *out,int verbose,int outcode,char ki[],char ko[]);
void removeescape(FILE *in,FILE *out,int verbose,int forcedelesc);
void printcode(int code);
int toup(int data);
int SkipESCSeq(FILE *in,int temp,int *intwobyte);
int DetectCodeType(FILE *in);
int getcode(char extension[],int data,char ki[],char ko[],int doing);
#ifdef THINK_C
int ccommand(char ***p);
#endif

void main(int argc,char **argv)
{
  FILE *in,*out;
#ifndef THINK_C
  int rc;
#endif
  int tempincode,incode,doing = FALSE,forcedelesc = FALSE;
  int makeoutfile = TRUE,outcode = NOTSET,verbose = FALSE,delesc = FALSE;
  int repairjis = FALSE,tofullsize = FALSE,setincode = FALSE,docheck = FALSE;
  char infilename[100],outfilename[100],extension[10],ki[10],ko[10],toolname[100];

#ifdef THINK_C
  argc = ccommand(&argv);
#endif

  strcpy(toolname,*argv);
  while (--argc > 0 && (*++argv)[0] == '-')
    switch (toup(*++argv[0])) {
      case 'C' :
        docheck = TRUE;
        break;
      case 'F' :
        tofullsize = TRUE;
        break;
      case 'H' :
        dohelp(toolname);
        break;
      case 'I' :
        setincode = TRUE;
        doing = INPUT;
        incode = getcode(extension,toup(*++argv[0]),ki,ko,doing);
        break;
      case 'O' :
        doing = OUTPUT;
        outcode = getcode(extension,toup(*++argv[0]),ki,ko,doing);
        break;
      case 'R' :
        repairjis = TRUE;
        doing = REPAIR;
        outcode = getcode(extension,toup(*++argv[0]),ki,ko,doing);
        break;
      case 'S' :
        delesc = TRUE;
        strcpy(extension,".rem");
        if (toup(*++argv[0]) == 'F')
          forcedelesc = TRUE;
        break;
      case 'T' :
        switch (toup(*++argv[0])) {
          case 'E' :
            doeuctable();
            break;
          case 'J' :
          case 'N' :
          case 'O' :
            dojistable();
            break;
          case 'S' :
            dosjistable();
            break;
          default :
            dojistable();
            dosjistable();
            doeuctable();
            break;
        }
        exit(0);
        break;
      case 'V' :
        verbose = TRUE;
        break;
      default :
        fprintf(stderr,"Illegal option \"-%c\"! Try using the \"-h\" option for help.\n",*argv[0]);
        fprintf(stderr,"Usage: %s [-options] [infile] [outfile]\nExiting...\n",toolname);
        exit(1);
        break;
    }
  if (repairjis && delesc) {
    fprintf(stderr,"Error! Both \"-r\" and \"-s\" options cannot be selected! Exiting...\n");
    exit(1);
  }
  if (outcode == NOTSET && !repairjis && !delesc) {
    strcpy(ki,DEFAULT_OKI);
    strcpy(ko,DEFAULT_OKO);
    strcpy(extension,DEFAULT_OS);
    outcode = DEFAULT_O;
  }
  if (argc == 0) {
#ifndef THINK_C
    rc = lseek(0,0,SEEK_CUR);
    if (rc == -1 && errno == ESPIPE && !setincode && !delesc && !docheck && !repairjis) {
      fprintf(stderr,"Cannot automatically detect input code from a pipe!\n");
      fprintf(stderr,"Try \"-c\" to determine input code, then apply it to \"iCODE\" option.\n");
      fprintf(stderr,"Exiting...\n");
      exit(1);
    }
#endif
    in = stdin;
    out = stdout;
  }
  else if (argc > 0) {
    if (argc == 1) {
      strcpy(infilename,*argv);
      if (strchr(*argv,PERIOD) != NULL)
        *strrchr(*argv,PERIOD) = '\0';
      strcpy(outfilename,*argv);
      strcat(outfilename,extension);
      if (!strcmp(infilename,outfilename)) {
        if (strchr(outfilename,PERIOD) != NULL)
          *strrchr(outfilename,PERIOD) = '\0';
        strcat(outfilename,"-");
        strcat(outfilename,extension);
      }
      if (verbose && !docheck)
        fprintf(stderr,"Output file will be named %s\n",outfilename);
    }
    else if (argc > 1) {
      strcpy(infilename,*argv++);
      if (*argv[0] == '-') {
        out = stdout;
        makeoutfile = FALSE;
      }
      else {
        strcpy(outfilename,*argv);
        if (!strcmp(infilename,outfilename)) {
          if (strchr(outfilename,PERIOD) != NULL)
            *strrchr(outfilename,PERIOD) = '\0';
          strcat(outfilename,"-");
          strcat(outfilename,extension);
        }
      }
    }
    if ((in = fopen(infilename,"r")) == NULL) {
      fprintf(stderr,"Cannot open %s! Exiting...\n",infilename);
      exit(1);
    }
    if (!docheck && makeoutfile)
      if ((out = fopen(outfilename,"w")) == NULL) {
        fprintf(stderr,"Cannot open %s! Exiting...\n",outfilename);
        exit(1);
      }
  }
  if (repairjis) {
    jisrepair(in,out,verbose,outcode,ki,ko);
    exit(0);
  }
  if (delesc) {
    removeescape(in,out,verbose,forcedelesc);
    exit(0);
  }
  tempincode = incode;
  if (setincode && verbose) {
    fprintf(stderr,"User-selected input code: ");
    printcode(incode);
  }
  if (verbose && !docheck) {
    fprintf(stderr,"User-selected output code: ");
    printcode(outcode);
  }
  if (setincode && verbose)
    ;
  else if (!setincode || docheck || verbose) {
    incode = DetectCodeType(in);
    if (docheck || verbose) {
      if (setincode && docheck)
        fprintf(stderr,"NOTE: The selected \"-iCODE\" option was ignored\n");
      fprintf(stderr,"Detected input code: ");
      printcode(incode);
      if (docheck)
        exit(0);
    }
    rewind(in);
  }
  if (setincode)
    incode = tempincode;
  switch (incode) {
    case NOTSET :
      fprintf(stderr,"Unknown input code! Exiting...\n");
      exit(1);
      break;
    case EUCORSJIS :
      fprintf(stderr,"Ambiguous (Shift-JIS or EUC) input code!\n");
      fprintf(stderr,"Try using the \"-iCODE\" option to specify either Shift-JIS or EUC.\n");
      fprintf(stderr,"Exiting...\n");
      exit(1);
      break;
    case ASCII :
      fprintf(stderr,"Since detected input code is ASCII, it may be damaged New- or Old-JIS\n");
      fprintf(stderr,"Trying to repair...\n");
      jisrepair(in,out,verbose,outcode,ki,ko);
      break;
    case NEW :
    case OLD :
    case NEC :
      switch (outcode) {
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
      switch (outcode) {
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
      switch (outcode) {
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
  }
  exit(0);
}

int toup(int data)
{
  if (islower(data))
    return (toupper(data));
  else
    return data;
}

void printcode(int code)
{
  switch (code) {
    case OLD :
      fprintf(stderr,"Old-JIS\n");
      break;
    case NEW :
      fprintf(stderr,"New-JIS\n");
      break;
    case NEC :
      fprintf(stderr,"NEC-JIS\n");
      break;
    case EUC :
      fprintf(stderr,"EUC\n");
      break;
    case SJIS :
      fprintf(stderr,"Shift-JIS\n");
      break;
    case EUCORSJIS :
      fprintf(stderr,"ambiguous (Shift-JIS or EUC)\n");
      break;
    case ASCII :
      fprintf(stderr,"ASCII (no Japanese)\n");
      break;
    case NOTSET :
      fprintf(stderr,"unknown\n");
      break;
    default :
      break;
  }
}

int getcode(char extension[],int data,char ki[],char ko[],int doing)
{
  if (data == 'E') {
    if (doing == OUTPUT || doing == REPAIR)
      strcpy(extension,".euc");
    return EUC;
  }
  else if (data == 'S') {
    if ((doing == OUTPUT) || (doing == REPAIR))
      strcpy(extension,".sjs");
    return SJIS;
  }
  else if (data == 'J') {
    if (doing == OUTPUT || doing == REPAIR) {
      strcpy(ki,"$B");
      strcpy(ko,"(J");
      strcpy(extension,".new");
    }
    return NEW;
  }
  else if (data == 'O') {
    if (doing == OUTPUT || doing == REPAIR) {
      strcpy(ki,"$@");
      strcpy(ko,"(J");
      strcpy(extension,".old");
    }
    return OLD;
  }
  else if (data == 'N') {
    if (doing == OUTPUT || doing == REPAIR) {
      strcpy(ki,"K");
      strcpy(ko,"H");
      strcpy(extension,".nec");
    }
    return NEC;
  }
  else {
    if (doing == INPUT) {
      fprintf(stderr,"Missing or invalid user-selected input code! Exiting...\n");
      exit(1);
    }
    else if (doing == OUTPUT) {
      strcpy(ki,DEFAULT_OKI);
      strcpy(ko,DEFAULT_OKO);
      strcpy(extension,DEFAULT_OS);
      return DEFAULT_O;
    }
    else if (doing == REPAIR) {
      strcpy(ki,DEFAULT_RKI);
      strcpy(ko,DEFAULT_RKO);
      strcpy(extension,DEFAULT_RS);
      return DEFAULT_R;
    }
  }
}

void dojistable(void)
{
  printf("JIS Code Specifications:\n");
  printf("                             DECIMAL                  HEXADECIMAL\n");
  printf("Two-byte character escape sequences\n");
  printf(" JIS C 6226-1978 (Old-JIS)   027 036 064              1B 24 40\n");
  printf(" JIS C 6226-1978 (NEC-JIS)   027 075                  1B 4B\n");
  printf(" JIS X 0208-1983 (New-JIS)   027 036 066              1B 24 42\n");
  printf(" JIS X 0208-1990             027 038 064 027 036 066  1B 26 40 1B 24 42\n");
  printf(" JIS X 0212-1990             027 036 040 068          1B 24 28 44\n");
  printf("Two-byte characters\n");
  printf(" first byte range            033-126                  21-7E\n");
  printf(" second byte range           033-126                  21-7E\n");
  printf("One-byte character escape sequences\n");
  printf(" JIS-Roman                   027 040 074              1B 28 4A\n");
  printf(" JIS-Roman (NEC)             027 072                  1B 48\n");
  printf(" ASCII                       027 040 066              1B 28 42\n");
  printf(" half-width katakana         027 040 073              1B 28 49\n");
  printf("JIS7 half-width katakana\n");
  printf(" byte range                  033-095                  21-5F\n");
  printf("JIS8 half-width katakana\n");
  printf(" shift-out                   014                      0E\n");
  printf(" byte range                  161-223                  A1-DF\n");
  printf(" shift-in                    015                      0F\n\n");
  printf("NOTE: This version of the tool does not support the escape sequences\n");
  printf("      for JIS X 0212-1990 and JIS X 0208-1990, nor any of the half-\n");
  printf("      width katakana specifications\n\n\n");
}

void doeuctable(void)
{
  printf("EUC Code Specifications (Packed format):\n");
  printf("                             DECIMAL                  HEXADECIMAL\n");
  printf("Code set 0 (ASCII)\n");
  printf(" byte range                  033-126                  21-7E\n");
  printf("Code set 1 (JIS X 0208-1990)\n");
  printf(" first byte range            161-254                  A1-FE\n");
  printf(" second byte range           161-254                  A1-FE\n");
  printf("Code set 2 (Half-width katakana)\n");
  printf(" first byte                  142                      8E\n");
  printf(" second byte range           161-223                  A1-DF\n");
  printf("Code set 3 (JIS X 0212-1990)\n");
  printf(" first byte                  143                      8F\n");
  printf(" second byte range           161-254                  A1-FE\n");
  printf(" third byte range            161-254                  A1-FE\n\n");
  printf("EUC Code Specifications (Complete 2-byte format):\n");
  printf("                             DECIMAL                  HEXADECIMAL\n");
  printf("Code set 0 (ASCII)\n");
  printf(" first byte                  000                      00\n");
  printf(" second byte range           033-126                  21-7E\n");
  printf("Code set 1 (JIS X 0208-1990)\n");
  printf(" first byte range            161-254                  A1-FE\n");
  printf(" second byte range           161-254                  A1-FE\n");
  printf("Code set 2 (Half-width katakana)\n");
  printf(" first byte                  000                      00\n");
  printf(" second byte range           161-223                  A1-DF\n");
  printf("Code set 3 (JIS X 0212-1990)\n");
  printf(" first byte range            161-254                  A1-FE\n");
  printf(" second byte range           033-126                  21-7E\n\n");
  printf("NOTE: This version of the tool does not support code set 3 nor\n");
  printf("      the Complete 2-byte format\n\n\n");
}

void dosjistable(void)
{
  printf("Shift-JIS Code Specifications:\n");
  printf("                             DECIMAL                  HEXADECIMAL\n");
  printf("Two-byte characters\n");
  printf(" first byte range            129-159, 224-239         81-9F, E0-EF\n");
  printf(" second byte range           064-126, 128-252         40-7E, 80-FC\n");
  printf("One-byte characters\n");
  printf(" Half-width katakana         161-223                  A1-DF\n");
  printf(" ASCII/JIS-Roman             033-126                  21-7E\n\n\n");
}

void dohelp(char toolname[])
{
  printf("** %s v3.0 (July 1, 1993) **\n\n",toolname);
  printf("Written by Ken R. Lunde, Adobe Systems Incorporated\nlunde@mv.us.adobe.com\n\n");
  printf("Usage: %s [-options] [infile] [outfile]\n\n",toolname);
  printf("Tool description: This tool is a utility for converting the Japanese code of\n");
  printf("textfiles, and supports Shift-JIS, EUC, New-JIS, Old-JIS, and NEC-JIS for\n");
  printf("both input and output. It can also display a file's input code, repair\n");
  printf("damaged Old- or New-JIS files, and display the specifications for any of the\n");
  printf("handled codes.\n\n");
  printf("Options include:\n\n");
  printf("  -c        Displays the detected input code, then exits -- the types\n");
  printf("            reported include EUC, Shift-JIS, New-JIS, Old-JIS, NEC-JIS, ASCII\n");
  printf("            (no Japanese), ambiguous (Shift-JIS or EUC), and unknown (note\n");
  printf("            that this option overrides \"-iCODE\")\n");
  printf("  -f        Converts half-width katakana to their full-width equivalents (this\n");
  printf("            option is forced when output code is New-, Old-, or NEC-JIS)\n");
  printf("  -h        Displays this help page, then exits\n");
  printf("  -iCODE    Forces input code to be recognized as CODE\n");
  printf("  -o[CODE]  Output code set to CODE (default is Shift-JIS if this option is\n");
  printf("            not specified, or if the specified CODE is invalid)\n");
  printf("  -r[CODE]  Repairs damaged New- and Old-JIS encoded files by restoring lost\n");
  printf("            escape characters, then converts it to the CODE specified (the\n");
  printf("            default is to convert the file to New-JIS if CODE is not\n");
  printf("            specified -- cannot be used in conjunction with \"-s\")\n");
  printf("  -s[f]     Removes escape characters from valid escape sequences of New- and\n");
  printf("            Old-JIS encoded files -- \"f\" will force all escape characters\n");
  printf("            to be removed (default extension is .rem -- cannot be used in\n");
  printf("            conjunction with \"-r\")\n");
  printf("  -t[CODE]  Prints a table listing the specifications for the specified CODE,\n");
  printf("            then exits (all code tables will be displayed if CODE is not\n");
  printf("            specified, or if CODE is invalid)\n");
  printf("  -v        Verbose mode -- displays information such as automatically\n");
  printf("            generated file names, detected input code, number of escape\n");
  printf("            characters restored/removed, etc.\n\n");
  printf("NOTE: CODE has five possible values (and default outfile extensions):\n");
  printf("      \"e\" = EUC (.euc); \"s\" = Shift-JIS (.sjs); \"j\" = New-JIS (.new);\n");
  printf("      \"o\" = Old-JIS (.old); and \"n\" = NEC-JIS (.nec)\n\n");
  exit(0);
}

int SkipESCSeq(FILE *in,int temp,int *intwobyte)
{
  int tempdata = *intwobyte;

  if (temp == '$' || temp == '(')
    fgetc(in);
  if (temp == 'K' || temp == '$')
    *intwobyte = TRUE;
  else
    *intwobyte = FALSE;
  if (tempdata == *intwobyte)
    return FALSE;
  else
    return TRUE;
}
 
void removeescape(FILE *in,FILE *out,int verbose,int forcedelesc)
{
  int p1,p2,p3;
  unsigned long count = 0,other = 0;

  while ((p1 = fgetc(in)) != EOF) {
    if (p1 == ESC) {
      p2 = fgetc(in);
      if (p2 == '(') {
        p3 = fgetc(in);
        switch (p3) {
          case 'J' :
          case 'B' :
          case 'H' :
            fprintf(out,"%c%c",p2,p3);
            count++;
            break;
          default :
            if (forcedelesc)
              fprintf(out,"%c%c",p2,p3);
            else
              fprintf(out,"%c%c%c",p1,p2,p3);
            other++;
            break;
        }
      }
      else if (p2 == '$') {
        p3 = fgetc(in);
        switch (p3) {
          case 'B' :
          case '@' :
            fprintf(out,"%c%c",p2,p3);
            count++;
            break;
          default :
            if (forcedelesc)
              fprintf(out,"%c%c",p2,p3);
            else
              fprintf(out,"%c%c%c",p1,p2,p3);
            other++;
            break;
        }
      }
      else {
        if (forcedelesc)
          fprintf(out,"%c",p2);
        else
          fprintf(out,"%c%c",p1,p2);
        other++;
      }
    }
    else
      fprintf(out,"%c",p1);
  }
  if (verbose) {
    fprintf(stderr,"Number of valid escape characters removed: %lu\n",count);
    if (forcedelesc)
      fprintf(stderr,"Number of other escape characters forced removed: %lu\n",other);
    else
      fprintf(stderr,"Number of other escape characters detected: %lu\n",other);
  }
}

void jisrepair(FILE *in,FILE *out,int verbose,int outcode,char ki[],char ko[])
{
  int p1,p2,p3,intwobyte = FALSE;
  unsigned long count = 0;

  while ((p1 = fgetc(in)) != EOF) {
    if (intwobyte) {
      if (p1 == ESC) {
        p2 = fgetc(in);
        if (p2 == '(') {
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
                  fprintf(out,"%c%s",ESC,ko);
                  break;
                default :
                  break;
              }
              break;
            default :
              fprintf(out,"%c%c%c",p1,p2,p3);
              break;
          }
        }
        else if (p2 == 'H') {
          intwobyte = FALSE;
          switch (outcode) {
            case NEC :
            case NEW :
            case OLD :
              fprintf(out,"%c%s",ESC,ko);
              break;
            default :
              break;
          }
        }
        else
          fprintf(out,"%c%c",p1,p2);
      }
      else if (p1 == '(') {
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
                fprintf(out,"%c%s",ESC,ko);
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
                fprintf(out,"%c%c",p1,p2);
                break;
              case EUC :
                p1 += 128;
                p2 += 128;
                fprintf(out,"%c%c",p1,p2);
                break;
              case SJIS :
                jis2sjis(&p1,&p2);
                fprintf(out,"%c%c",p1,p2);
                break;
            }
            break;
        }
      }
      else if (p1 == NL) {
        switch (outcode) {
          case NEC :
          case NEW :
          case OLD :
            fprintf(out,"%c%s%c",ESC,ko,p1);
            break;
          default :
            fprintf(out,"%c",p1);
            break;
        }
        count++;
        intwobyte = FALSE;
      }
      else {
        p2 = fgetc(in);
        switch (outcode) {
          case NEC :
          case NEW :
          case OLD :
            fprintf(out,"%c%c",p1,p2);
            break;
          case EUC :
            p1 += 128;
            p2 += 128;
            fprintf(out,"%c%c",p1,p2);
            break;
          case SJIS :
            jis2sjis(&p1,&p2);
            fprintf(out,"%c%c",p1,p2);
            break;
        }
      }
    }
    else {
      if (p1 == ESC) {
        p2 = fgetc(in);
        if (p2 == '$') {
          p3 = fgetc(in);
          switch (p3) {
            case 'B' :
            case '@' :
              intwobyte = TRUE;
              switch (outcode) {
                case NEC :
                case NEW :
                case OLD :
                  fprintf(out,"%c%s",ESC,ki);
                  break;
                default :
                  break;
              }
              break;
            default :
              fprintf(out,"%c%c%c",p1,p2,p3);
              break;
          }
        }
        else if (p2 == 'K') {
          intwobyte = TRUE;
          switch (outcode) {
            case NEC :
            case NEW :
            case OLD :
              fprintf(out,"%c%s",ESC,ki);
              break;
            default :
              break;
          }
        }
        else
          fprintf(out,"%c%c",p1,p2);
      }
      else if (p1 == '$') {
        p2 = fgetc(in);
        switch (p2) {
          case 'B' :
          case '@' :
            intwobyte = TRUE;
            switch (outcode) {
              case NEC :
              case NEW :
              case OLD :
                fprintf(out,"%c%s",ESC,ki);
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
                fprintf(out,"%c%c",p1,p2);
                break;
              case EUC :
                fprintf(out,"%c%c",p1,p2);
                break;
              case SJIS :
                fprintf(out,"%c%c",p1,p2);
                break;
            }
            break;
        }
      }
      else
        fprintf(out,"%c",p1);
    }
  }
  if (intwobyte) {
    switch (outcode) {
      case NEC :
      case NEW :
      case OLD :
        fprintf(out,"%c%s",ESC,ko);
        count++;
        break;
      default :
        break;
    }
  }
  if (verbose)
    fprintf(stderr,"Number of escape characters restored: %lu\n",count);
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

void shift2seven(FILE *in,FILE *out,int incode,char ki[],char ko[])
{
  int p1,p2,intwobyte = FALSE;

  while ((p1 = fgetc(in)) != EOF) {
    if (p1 == NL || p1 == CR) {
      if (intwobyte) {
        intwobyte = FALSE;
        fprintf(out,"%c%s",ESC,ko);
      }
      fprintf(out,"%c",p1);
    }
    else if (SJIS1(p1)) {
      p2 = fgetc(in);
      if (SJIS2(p2)) {
        sjis2jis(&p1,&p2);
        if (!intwobyte) {
          intwobyte = TRUE;
          fprintf(out,"%c%s",ESC,ki);
        }
      }
      fprintf(out,"%c%c",p1,p2);
    }
    else if (HANKATA(p1)) {
      han2zen(in,&p1,&p2,incode);
      sjis2jis(&p1,&p2);
      if (!intwobyte) {
        intwobyte = TRUE;
        fprintf(out,"%c%s",ESC,ki);
      }
      fprintf(out,"%c%c",p1,p2);
    }
    else {
      if (intwobyte) {
        intwobyte = FALSE;
        fprintf(out,"%c%s",ESC,ko);
      }
      fprintf(out,"%c",p1);
    }
  }
  if (intwobyte)
    fprintf(out,"%c%s",ESC,ko);
}

void shift2euc(FILE *in,FILE *out,int incode,int tofullsize)
{
  int p1,p2;
  
  while ((p1 = fgetc(in)) != EOF) {
    if (SJIS1(p1)) {
      p2 = fgetc(in);
      if (SJIS2(p2)) {
        sjis2jis(&p1,&p2);
        p1 += 128;
        p2 += 128;
      }
      fprintf(out,"%c%c",p1,p2);
    }
    else if (HANKATA(p1)) {
      if (tofullsize) {
        han2zen(in,&p1,&p2,incode);
        sjis2jis(&p1,&p2);
        p1 += 128;
        p2 += 128;
      }
      else {
        p2 = p1;
        p1 = SS2;
      }
      fprintf(out,"%c%c",p1,p2);
    }
    else
      fprintf(out,"%c",p1);
  }
}
 
void euc2seven(FILE *in,FILE *out,int incode,char ki[],char ko[])
{
  int p1,p2,intwobyte = FALSE;

  while ((p1 = fgetc(in)) != EOF) {
    if (p1 == NL || p1 == CR) {
      if (intwobyte) {
        intwobyte = FALSE;
        fprintf(out,"%c%s",ESC,ko);
      }
      fprintf(out,"%c",p1);
    }
    else {
      if (ISEUC(p1)) {
        p2 = fgetc(in);
        if (ISEUC(p2)) {
          p1 -= 128;
          p2 -= 128;
          if (!intwobyte) {
            intwobyte = TRUE;
            fprintf(out,"%c%s",ESC,ki);
          }
        }
        fprintf(out,"%c%c",p1,p2);
      }
      else if (p1 == SS2) {
        p2 = fgetc(in);
        if (HANKATA(p2)) {
          p1 = p2;
          han2zen(in,&p1,&p2,incode);
          sjis2jis(&p1,&p2);
          if (!intwobyte) {
            intwobyte = TRUE;
            fprintf(out,"%c%s",ESC,ki);
          }
        }
        fprintf(out,"%c%c",p1,p2);
      }
      else {
        if (intwobyte) {
          intwobyte = FALSE;
          fprintf(out,"%c%s",ESC,ko);
        }
        fprintf(out,"%c",p1);
      }
    }
  }
  if (intwobyte)
    fprintf(out,"%c%s",ESC,ko);
}
 
void euc2shift(FILE *in,FILE *out,int incode,int tofullsize)
{
  int p1,p2;

  while ((p1 = fgetc(in)) != EOF) {
    if (ISEUC(p1)) {
      p2 = fgetc(in);
      if (ISEUC(p2)) {
        p1 -= 128;
        p2 -= 128;
        jis2sjis(&p1,&p2);
      }
      fprintf(out,"%c%c",p1,p2);
    }
    else if (p1 == SS2) {
      p2 = fgetc(in);
      if (HANKATA(p2)) {
        p1 = p2;
        if (tofullsize) {
          han2zen(in,&p1,&p2,incode);
          fprintf(out,"%c%c",p1,p2);
        }
        else {
          fprintf(out,"%c",p1);
        }
      }
      else
        fprintf(out,"%c%c",p1,p2);
    }
    else
      fprintf(out,"%c",p1);
  }
}

void euc2euc(FILE *in,FILE *out,int incode,int tofullsize)
{
  int p1,p2;

  while ((p1 = fgetc(in)) != EOF) {
    if (ISEUC(p1)) {
      p2 = fgetc(in);
      if (ISEUC(p2))
        fprintf(out,"%c%c",p1,p2);
    }
    else if (p1 == SS2) {
      p2 = fgetc(in);
      if (HANKATA(p2) && tofullsize) {
        p1 = p2;
        han2zen(in,&p1,&p2,incode);
        sjis2jis(&p1,&p2);
        p1 += 128;
        p2 += 128;
      }
      fprintf(out,"%c%c",p1,p2);
    }
    else
      fprintf(out,"%c",p1);
  }
}

void shift2shift(FILE *in,FILE *out,int incode,int tofullsize)
{
  int p1,p2;
  
  while ((p1 = fgetc(in)) != EOF) {
    if (SJIS1(p1)) {
      p2 = fgetc(in);
      if (SJIS2(p2))
        fprintf(out,"%c%c",p1,p2);
    }
    else if (HANKATA(p1) && tofullsize) {
      han2zen(in,&p1,&p2,incode);
      fprintf(out,"%c%c",p1,p2);
    }
    else
      fprintf(out,"%c",p1);
  }
}

void seven2shift(FILE *in,FILE *out)
{
  int temp,p1,p2,intwobyte = FALSE;

  while ((p1 = fgetc(in)) != EOF) {
    if (p1 == ESC) {
      temp = fgetc(in);
      SkipESCSeq(in,temp,&intwobyte);
    }
    else if (p1 == NL || p1 == CR) {
      if (intwobyte)
        intwobyte = FALSE;
      fprintf(out,"%c",p1);
    }
    else {
      if (intwobyte) {
        p2 = fgetc(in);
        jis2sjis(&p1,&p2);
        fprintf(out,"%c%c",p1,p2);
      }
      else
        fprintf(out,"%c",p1);
    }
  }
}
  
void seven2euc(FILE *in,FILE *out)
{
  int temp,p1,p2,intwobyte = FALSE;

  while ((p1 = fgetc(in)) != EOF) {
    if (p1 == ESC) {
      temp = fgetc(in);
      SkipESCSeq(in,temp,&intwobyte);
    }
    else if (p1 == NL || p1 == CR) {
      if (intwobyte)
        intwobyte = FALSE;
      fprintf(out,"%c",p1);
    }
    else {
      if (intwobyte) {
        p2 = fgetc(in);
        p1 += 128;
        p2 += 128;
        fprintf(out,"%c%c",p1,p2);
      }
      else
        fprintf(out,"%c",p1);
    }
  }
}

void seven2seven(FILE *in,FILE *out,char ki[],char ko[])
{
  int temp,p1,p2,change,intwobyte = FALSE;

  while ((p1 = fgetc(in)) != EOF) {
    if (p1 == ESC) {
      temp = fgetc(in);
      change = SkipESCSeq(in,temp,&intwobyte);
      if ((intwobyte) && (change))
        fprintf(out,"%c%s",ESC,ki);
      else if (change)
        fprintf(out,"%c%s",ESC,ko);
    }
    else if (p1 == NL || p1 == CR) {
      if (intwobyte) {
        intwobyte = FALSE;
        fprintf(out,"%c%s",ESC,ko);
      }
      fprintf(out,"%c",p1);
    }
    else {
      if (intwobyte) {
        p2 = fgetc(in);
        fprintf(out,"%c%c",p1,p2);
      }
      else
        fprintf(out,"%c",p1);
    }
  }
  if (intwobyte)
    fprintf(out,"%c%s",ESC,ko);
}

int DetectCodeType(FILE *in)
{
  int c = 0,whatcode = ASCII;

  while ((whatcode == EUCORSJIS || whatcode == ASCII) && c != EOF) {
    if ((c = fgetc(in)) != EOF) {
      if (c == ESC) {
        c = fgetc(in);
        if (c == '$') {
          c = fgetc(in);
          if (c == 'B')
            whatcode = NEW;
          else if (c == '@')
            whatcode = OLD;
        }
        else if (c == 'K')
          whatcode = NEC;
      }
      else if ((c >= 129 && c <= 141) || (c >= 143 && c <= 159))
        whatcode = SJIS;
      else if (c == SS2) {
        c = fgetc(in);
        if ((c >= 64 && c <= 126) || (c >= 128 && c <= 160) || (c >= 224 && c <= 252))
          whatcode = SJIS;
        else if (c >= 161 && c <= 223)
          whatcode = EUCORSJIS;
      }
      else if (c >= 161 && c <= 223) {
        c = fgetc(in);
        if (c >= 240 && c <= 254)
          whatcode = EUC;
        else if (c >= 161 && c <= 223)
          whatcode = EUCORSJIS;
        else if (c >= 224 && c <= 239) {
          whatcode = EUCORSJIS;
          while (c >= 64 && c != EOF && whatcode == EUCORSJIS) {
            if (c >= 129) {
              if (c <= 141 || (c >= 143 && c <= 159))
                whatcode = SJIS;
              else if (c >= 253 && c <= 254)
                whatcode = EUC;
            }
            c = fgetc(in);
          }
        }
        else if (c <= 159)
          whatcode = SJIS;
      }
      else if (c >= 240 && c <= 254)
        whatcode = EUC;
      else if (c >= 224 && c <= 239) {
        c = fgetc(in);
        if ((c >= 64 && c <= 126) || (c >= 128 && c <= 160))
          whatcode = SJIS;
        else if (c >= 253 && c <= 254)
          whatcode = EUC;
        else if (c >= 161 && c <= 252)
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

  if (incode == SJIS) {
    *p2 = fgetc(in);
    if (*p2 == 222) {
      if (ISNIGORI(*p1))
        nigori = TRUE;
      else
        ungetc(*p2,in);
    }
    else if (*p2 == 223) {
      if (ISMARU(*p1))
        maru = TRUE;
      else
        ungetc(*p2,in);
    }
    else
      ungetc(*p2,in);
  }
  else if (incode == EUC) {
    junk = fgetc(in);
    if (junk == SS2) {
      *p2 = fgetc(in);
      if (*p2 == 222) {
        if (ISNIGORI(*p1))
          nigori = TRUE;
        else {
          ungetc(*p2,in);
          ungetc(junk,in);
        }
      }
      else if (*p2 == 223) {
        if (ISMARU(*p1))
          maru = TRUE;
        else {
          ungetc(*p2,in);
          ungetc(junk,in);
        }
      }
      else {
        ungetc(*p2,in);
        ungetc(junk,in);
      }
    }
    else
      ungetc(junk,in);
  }
  if (*p1 >= 161 && *p1 <= 223) {
    *p1 = mtable[tmp - 161][0];
    *p2 = mtable[tmp - 161][1];
  }
  if (nigori) {
    if ((*p2 >= 74 && *p2 <= 103) || (*p2 >= 110 && *p2 <= 122))
      (*p2)++;
    else if (*p1 == 131 && *p2 == 69)
      *p2 = 148;
  }
  else if (maru && *p2 >= 110 && *p2 <= 122)
    *p2 += 2;
}
