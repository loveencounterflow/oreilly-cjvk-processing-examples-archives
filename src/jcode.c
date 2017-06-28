/*
Program: jcode.c
Version: 3.0
Date:    July 1, 1993
Author:  Ken R. Lunde, Adobe Systems Incorporated
  EMAIL: lunde@mv.us.adobe.com
  MAIL : 1585 Charleston Road, P.O. Box 7900, Mountain View, CA 94039-7900
Type:    A tool for displaying the electronic values of Japanese characters.
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
 1) Supports Shift-JIS, EUC, New-JIS, Old-JIS, and NEC-JIS codes.
 2) Automatically detects infile's Japanese code.
 3) The tool reads the infile, and will create an outfile which
    contains all of the characters within the infile along with their
    electronic codes. The user may choose octal, decimal, or hexadecimal
    notation (the default, if none specified, is hexadecimal).
 4) Supports half-width katakana in both Shift-JIS and EUC codes as input.
 5) The outfile specifies the Shift-JIS, EUC, JIS, ASCII, and KUTEN values
    for each character in the file. The default file extension is ".val" if
    no outfile is specified.

FILE HANDLING:
 1) Specifying the infile is optional as one can redirect stdin and stdout.
 2) Specifying the outfile is also optional. If none is specified, the
    tool will semi-intelligently change the file's name. The tool
    simply scans the outfile name, finds the last period in it, terminates
    the string at that point, and tacks on an extension. Here are some example
    command lines, and the resulting outfile names:

    a) jcode sig.jpn                      = sig.val
    b) jcode sig.jpn sig.out              = sig.out
    c) jcode sig.jpn.txt                  = sig.jpn.val
    d) jcode sig                          = sig.val

    This is very useful for MS-DOS users since a filename such as sig.jpn.val
    will not result in converting a file called sig.jpn.

    Also note that if the outfile and infile have the same name, the tool
    will not work, and data will be lost. I tried to build safe-guards against
    this. For example, note how this tool will change the outfile name so
    that it does not overwrite the infile:
    
    a) jcode sig.val                      = sig-.val
    b) jcode sig.val sig.val              = sig-.val
    c) jcode sig-.val                     = sig--.val
    
    If only the infile is given, a hyphen is inserted after the last period,
    and the extension is then reattached. If the outfile is specified by the
    user, then it will search for the last period (if any), attach a hyphen,
    and finally attach the proper extension). This sort of protection is NOT
    available from this tool if stdin/stdout are used.
 3) If you want to specify an infile, but to have the output to go to stdout,
    you have two options:

    a) jcode < sig.jpn                    <= redirect stdin for infile
    b) jcode sig.jpn -                    <= specify infile, then use "-" to
                                             indictate stdout for output

 4) When using pipes under UNIX, it is very important to use the "-iCODE"
    option so that the automatic code detection routines are skipped. This
    is because the tool attempts to use the rewind() function, and this
    has no effect on some machines if input is coming from a pipe. Here
    are some examples (they assume that the infile code is EUC):

    a) cat japan.txt | jcode -oj          <= does not work
    a) cat japan.txt | jcode -ie -oj      <= works
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

#define INPUT        1
#define OUTPUT       2
#define NOTSET       0
#define NEW          1
#define OLD          2
#define NEC          3
#define EUC          4
#define SJIS         5
#define EUCORSJIS    6
#define ASCII        7
#define KUTEN        8
#define JISORSJIS    9
#define TRUE         1
#define FALSE        0
#define ESC          27
#define SJS1         0
#define SJS2         1
#define EUC1         2
#define EUC2         3
#define JIS1         4
#define JIS2         5
#define KT1          6
#define KT2          7
#define OCT          8
#define DEC          10
#define HEX          16
#define ASCII_ZERO   48
#define KUTEN_OFFSET 32
#define EUC_OFFSET   128
#define PERIOD       '.'
#define SJIS1(A)     ((A >= 129 && A <= 159) || (A >= 224 && A <= 239))
#define SJIS2(A)     (A >= 64 && A <= 252)
#define ISJIS(A)     (A >= 33 && A <= 126)
#define ISKUTEN(A)   (A >= 1 && A <= 94)
#define HANKATA(A)   (A >= 161 && A <= 223)
#define ISEUC(A)     (A >= 161 && A <= 254)
#define NOTEUC(A,B)  ((A >= 129 && A <= 159) && (B >= 64 && B <= 160))
#define ISASCII(A)   (A >= 0 && A <= 127)
#define VALID(X)     ((X >= '0' && X <= '9') || (X >= 'A' && X <= 'F'))
#ifndef SEEK_CUR
#define SEEK_CUR     1
#endif
/* The following 4 lines of code are used to establish the default output code
 * when using the "-o[CODE]" option, and for the default notation when using
 * the "-n[NOTATION]" option. They are self-explanatory, and easy to change.
 */
#define DEFAULT_O    SJIS     /* default output code for "-o[CODE]" option */
#define DEFAULT_OKI  ""       /* default kanji-in code for "-o[CODE]" option */
#define DEFAULT_OKO  ""       /* default kanji-out code for "-o[CODE]" option */
#define DEFAULT_N    HEX      /* default notation for "-n[NOTATION]" option */

void printheader(FILE *out,int pad);
void print1byte(FILE *out,int notation,int p1,int control,int pad);
void print2byte(FILE *out,int outcode,int notation,int data[],char ki[],char ko[],int pad);
void printhwkana(FILE *out,int outcode,int notation,int p1,int p2,int pad);
void jis2sjis(int *p1,int *p2);
void sjis2jis(int *p1,int *p2);
void TreatCodes(FILE *in,FILE *out,int outcode,int notation,int pad,char ki[],char ko[]);
void TreatEUC(FILE *in,FILE *out,int outcode,int notation,char ki[],char ko[],int control,int pad);
void TreatJIS(FILE *in,FILE *out,int outcode,int notation,char ki[],char ko[],int control,int pad);
void TreatSJIS(FILE *in,FILE *out,int outcode,int notation,char ki[],char ko[],int control,int pad);
void TreatASCII(FILE *in,FILE *out,int notation,int control,int pad);
void printcode(int code);
void dohelp(char toolname[]);
void TreatOneCode(char mydata[]);
int toup(int data);
int SkipESCSeq(FILE *in,int temp,int *intwobyte);
int DetectCodeType(FILE *in);
int getline(FILE *in,char *myarray);
int getcode(int data,char ki[],char ko[],int doing);
int getnotation(int data);
int hex2dec(int data);
int convert2code(int p1,int p2,int incode);
int dopadding(int data);
#ifdef THINK_C
int ccommand(char ***p);
#endif

void main(int argc,char **argv)
{
  FILE *in,*out;
#ifndef THINK_C
  int rc;
#endif
  int tempincode,incode,verbose = FALSE,makeoutfile = TRUE,doing = FALSE,control = FALSE;
  int outcode = NOTSET,notation = DEFAULT_N,pad = TRUE,setincode = FALSE,readcodes = FALSE;
  char infilename[100],outfilename[100],extension[10],ki[10],ko[10],toolname[100];

#ifdef THINK_C
  argc = ccommand(&argv);
#endif

  strcpy(toolname,*argv);
  while (--argc > 0 && (*++argv)[0] == '-')
    switch (toup(*++argv[0])) {
      case 'C' :
        readcodes = TRUE;
        if (*++argv[0] != '\0') {
          TreatOneCode(argv[0]);
          exit(1);
        }
        break;
      case 'H' :
        dohelp(toolname);
        break;
      case 'I' :
        setincode = TRUE;
        doing = INPUT;
        incode = getcode(toup(*++argv[0]),ki,ko,doing);
        break;
      case 'N' :
        notation = getnotation(toup(*++argv[0]));
        break;
      case 'O' :
        doing = OUTPUT;
        outcode = getcode(toup(*++argv[0]),ki,ko,doing);
        break;
      case 'P' :
        pad = dopadding(toup(*++argv[0]));
        break;
      case 'S' :
        control = TRUE;
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
  if (argc == 0) {
#ifndef THINK_C
    rc = lseek(0,0,SEEK_CUR);
    if (rc == -1 && errno == ESPIPE && !setincode) {
      fprintf(stderr,"Cannot automatically detect input code from a pipe!\n");
      fprintf(stderr,"Try using the \"iCODE\" option.\nExiting...\n");
      exit(1);
    }
#endif
    in = stdin;
    out = stdout;
  }
  else if (argc > 0) {
    if (argc == 1) {
      strcpy(extension,".val");
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
      if (verbose)
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
    if (makeoutfile)
      if ((out = fopen(outfilename,"w")) == NULL) {
        fprintf(stderr,"Cannot open %s! Exiting...\n",outfilename);
        exit(1);
      }
  }
  if (readcodes && control)
    control = FALSE;
  if (outcode == NOTSET) {
    strcpy(ki,DEFAULT_OKI);
    strcpy(ko,DEFAULT_OKO);
    outcode = DEFAULT_O;
  }
  tempincode = incode;
  if (setincode && verbose) {
    fprintf(stderr,"User-selected input code: ");
    printcode(incode);
  }
  if (verbose) {
    fprintf(stderr,"User-selected output code: ");
    printcode(outcode);
    fprintf(stderr,"User-selected notation: ");
    switch (notation) {
      case OCT :
        fprintf(stderr,"octal\n");
        break;
      case DEC :
        fprintf(stderr,"decimal\n");
        break;
      case HEX :
        fprintf(stderr,"hexadecimal\n");
        break;
    }
  }
  if ((setincode && verbose) || readcodes)
    ;
  else if (!setincode || verbose) {
    incode = DetectCodeType(in);
    if (verbose) {
      fprintf(stderr,"Detected input code: ");
      printcode(incode);
    }
    rewind(in);
  }
  if (setincode)
    incode = tempincode;
  if ((incode != NOTSET && incode != EUCORSJIS) || readcodes)
    printheader(out,pad);
  if (readcodes)
    TreatCodes(in,out,outcode,notation,pad,ki,ko);
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
    case NEW :
    case OLD :
    case NEC :
      TreatJIS(in,out,outcode,notation,ki,ko,control,pad);
      break;
    case EUC :
      TreatEUC(in,out,outcode,notation,ki,ko,control,pad);
      break;
    case SJIS :
      TreatSJIS(in,out,outcode,notation,ki,ko,control,pad);
      break;
    case ASCII :
      TreatASCII(in,out,notation,control,pad);
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

int dopadding(int data)
{
  if (data == 'T')
    return FALSE;
  else
    return TRUE;
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

void dohelp(char toolname[])
{
  printf("** %s v3.0 (July 1, 1993) **\n\n",toolname);
  printf("Written by Ken R. Lunde, Adobe Systems Incorporated\nlunde@mv.us.adobe.com\n\n");
  printf("Usage: %s [-options] [infile] [outfile]\n\n",toolname);
  printf("Tool description: This tool is a utility for displaying the electronic values\n");
  printf("of Japanese characters within textfiles, and supports Shift-JIS, EUC, New-JIS,\n");
  printf("Old-JIS, and NEC-JIS for both input and output.\n\n");
  printf("Options include:\n\n");
  printf("  -c[DATA]      Reads codes, one per line, rather than characters as input --\n");
  printf("                if DATA is specified, only that code is treated, then exits\n");
  printf("                (KUTEN codes must be prefixed with \"k,\" and EUC codes with\n");
  printf("                \"x\" -- EUC, JIS, and Shift-JIS codes must be hexadecimal)\n");
  printf("  -h            Displays this help page, then exits\n");
  printf("  -iCODE        Forces input code to be recognized as CODE\n");
  printf("  -n[NOTATION]  Output notation set to NOTATION (default is hexadecimal if this\n");
  printf("                option is not specified, if NOTATION is not specified, or if\n");
  printf("                the specified NOTATION is invalid)\n");
  printf("  -o[CODE]      Output code set to CODE (default is Shift-JIS if this option\n");
  printf("                is not specified, if CODE is not specified, or if the\n");
  printf("                specified CODE is invalid)\n");
  printf("  -p[CHOICE]    Pads the columns with CHOICE whereby CHOICE can be either \"t\"\n");
  printf("                for tabs or \"s\" for spaces (default is spaces if this option\n");
  printf("                is not specified, if CHOICE is not specified, or if the\n");
  printf("                specified CHOICE is invalid)\n");
  printf("  -s            Shows control characters (except escape sequences)\n");
  printf("  -v            Verbose mode -- displays information such as automatically\n");
  printf("                generated file names, detected input code, etc.\n\n");
  printf("NOTE: CODE has five possible values:\n");
  printf("      \"e\" = EUC; \"s\" = Shift-JIS; \"j\" = New-JIS; \"o\" = Old-JIS;\n");
  printf("      and \"n\" = NEC-JIS\n\n");
  printf("NOTE: NOTATION has three possible values:\n");
  printf("      \"o\" = octal; \"d\" = decimal; and \"h\" = hexadecimal\n\n");
  exit(0);
}

int getnotation(int data)
{
  if (data == 'D')
    return DEC;
  else if (data == 'O')
    return OCT;
  else if (data == 'H')
    return HEX;
  else
    return DEFAULT_N;
}

int getcode(int data,char ki[],char ko[],int doing)
{
  if (data == 'E')
    return EUC;
  else if (data == 'S')
    return SJIS;
  else if (data == 'J') {
    if (doing == OUTPUT) {
      strcpy(ki,"\033$B");
      strcpy(ko,"\033(J");
    }
    return NEW;
  }
  else if (data == 'O') {
    if (doing == OUTPUT) {
      strcpy(ki,"\033$@");
      strcpy(ko,"\033(J");
    }
    return OLD;
  }
  else if (data == 'N') {
    if (doing == OUTPUT) {
      strcpy(ki,"\033K");
      strcpy(ko,"\033H");
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
      return DEFAULT_O;
    }
  }
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
      else if (c == 142) {
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

void printheader(FILE *out,int pad)
{
  if (pad)
    fprintf(out,"Character  Shift-JIS  EUC      JIS      ASCII  KUTEN\n");
  else
    fprintf(out,"Character\tShift-JIS\tEUC\tJIS\tASCII\tKUTEN\n");
}

void printhwkana(FILE *out,int outcode,int notation,int p1,int p2,int pad)
{
  if (!pad) {
    if (outcode == EUC)
      fprintf(out,"%c%c\t",p1,p2);
    else
      fprintf(out,"%c\t",p2);
    switch (notation) {
      case OCT :
        fprintf(out,"%03o\t%03o-%03o\t%03o\n",p2,p1,p2,p2 - 128);
        break;
      case DEC :
        fprintf(out,"%03d\t%03d-%03d\t%03d\n",p2,p1,p2,p2 - 128);
        break;
      case HEX :
        fprintf(out,"%02X\t%02X-%02X\t%02X\n",p2,p1,p2,p2 - 128);
        break;
    }
  }
  else {
    if (outcode == EUC)
      fprintf(out,"%c%c          ",p1,p2);
    else
      fprintf(out,"%c          ",p2);
    switch (notation) {
      case OCT :
        fprintf(out,"%03o        %03o-%03o  %03o\n",p2,p1,p2,p2 - 128);
        break;
      case DEC :
        fprintf(out,"%03d        %03d-%03d  %03d\n",p2,p1,p2,p2 - 128);
        break;
      case HEX :
        fprintf(out,"%02X         %02X-%02X    %02X\n",p2,p1,p2,p2 - 128);
        break;
    }
  }
}

void print1byte(FILE *out,int notation,int p1,int control,int pad)
{
  static char *controlchar[] = {
  "<NUL>","<SOH>","<STX>","<ETX>","<EOT>","<ENQ>","<ACK>","<BEL>","<BS> ","<HT> ",
  "<LF> ","<VT> ","<FF> ","<CR> ","<SO> ","<SI> ","<DLE>","<DC1>","<DC2>","<DC3>",
  "<DC4>","<NAK>","<SYN>","<ETB>","<CAN>","<EM> ","<SUB>","<ESC>","<FS> ","<GS> ",
  "<RS> ","<US> ","<SP> " };

  if (control) {
    if (p1 >= 0 && p1 <= 32) {
      fprintf(out,"%s",controlchar[p1]);
      if (pad)
        fprintf(out,"      ");
      else
        fprintf(out,"\t");
    }
    else if (p1 == 127) {
      fprintf(out,"<DEL>");
      if (pad)
        fprintf(out,"      ");
      else
        fprintf(out,"\t");
    }
    else {
      fprintf(out,"%c",p1);
      if (pad)
        fprintf(out,"          ");
      else
        fprintf(out,"\t");
    }


    switch (notation) {
      case OCT :
        fprintf(out,"%03o\n",p1);
        break;
      case DEC :
        fprintf(out,"%03d\n",p1);
        break;
      case HEX :
        fprintf(out,"%02X\n",p1);
        break;
    }
  }
  else if (!control && p1 >= 33 && p1 <= 126) {
    fprintf(out,"%c",p1);
    if (pad)
      fprintf(out,"          ");
    else
      fprintf(out,"\t");
    switch (notation) {
      case OCT :
        fprintf(out,"%03o\n",p1);
        break;
      case DEC :
        fprintf(out,"%03d\n",p1);
        break;
      case HEX :
        fprintf(out,"%02X\n",p1);
        break;
    }
  }
}

void print2byte(FILE *out,int outcode,int notation,int data[],char ki[],char ko[],int pad)
{
  switch (outcode) {
    case NEW :
    case OLD :
    case NEC :
      fprintf(out,"%s%c%c%s",ki,data[JIS1],data[JIS2],ko);
      break;
    case EUC :
      fprintf(out,"%c%c",data[EUC1],data[EUC2]);
      break;
    case SJIS :
      fprintf(out,"%c%c",data[SJS1],data[SJS2]);
      break;
    default :
      break;
  }
  if (!pad) {
    fprintf(out,"\t");
    switch (notation) {
      case OCT :
        fprintf(out,"%03o-%03o\t",data[SJS1],data[SJS2]);
        fprintf(out,"%03o-%03o\t",data[EUC1],data[EUC2]);
        fprintf(out,"%03o-%03o\t",data[JIS1],data[JIS2]);
        break;
      case DEC :
        fprintf(out,"%03d-%03d\t",data[SJS1],data[SJS2]);
        fprintf(out,"%03d-%03d\t",data[EUC1],data[EUC2]);
        fprintf(out,"%03d-%03d\t",data[JIS1],data[JIS2]);
        break;
      case HEX :
        fprintf(out,"%02X-%02X\t",data[SJS1],data[SJS2]);
        fprintf(out,"%02X-%02X\t",data[EUC1],data[EUC2]);
        fprintf(out,"%02X-%02X\t",data[JIS1],data[JIS2]);
        break;
    }
    fprintf(out,"%c%c\t",data[JIS1],data[JIS2]);
    fprintf(out,"%02d-%02d\n",data[KT1],data[KT2]);
  }
  else {
    fprintf(out,"         ");
    switch (notation) {
      case OCT :
        fprintf(out,"%03o-%03o    ",data[SJS1],data[SJS2]);
        fprintf(out,"%03o-%03o  ",data[EUC1],data[EUC2]);
        fprintf(out,"%03o-%03o  ",data[JIS1],data[JIS2]);
        break;
      case DEC :
        fprintf(out,"%03d-%03d    ",data[SJS1],data[SJS2]);
        fprintf(out,"%03d-%03d  ",data[EUC1],data[EUC2]);
        fprintf(out,"%03d-%03d  ",data[JIS1],data[JIS2]);
        break;
      case HEX :
        fprintf(out,"%02X-%02X      ",data[SJS1],data[SJS2]);
        fprintf(out,"%02X-%02X    ",data[EUC1],data[EUC2]);
        fprintf(out,"%02X-%02X    ",data[JIS1],data[JIS2]);
        break;
    }
    fprintf(out,"%c%c     ",data[JIS1],data[JIS2]);
    fprintf(out,"%02d-%02d\n",data[KT1],data[KT2]);
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

void TreatEUC(FILE *in,FILE *out,int outcode,int notation,char ki[],char ko[],int control,int pad)
{
  int p1,p2;
  int data[8];

  while ((p1 = fgetc(in)) != EOF) {
    if ISEUC(p1) {
      p2 = fgetc(in);
      if ISEUC(p2) {
        data[SJS1] = data[EUC1] = data[JIS1] = data[KT1] = p1;
        data[SJS2] = data[EUC2] = data[JIS2] = data[KT2] = p2;
        data[SJS1] -= EUC_OFFSET;
        data[SJS2] -= EUC_OFFSET;
        jis2sjis(&data[SJS1],&data[SJS2]);
        data[JIS1] -= EUC_OFFSET;
        data[JIS2] -= EUC_OFFSET;
        data[KT1] -= 160;
        data[KT2] -= 160;
        print2byte(out,outcode,notation,data,ki,ko,pad);
      }
      else {
        print1byte(out,notation,p1,control,pad);
        print1byte(out,notation,p2,control,pad);
      }
    }
    else if ISASCII(p1)
      print1byte(out,notation,p1,control,pad);
    else if (p1 == 142) {
      p2 = fgetc(in);
      printhwkana(out,outcode,notation,p1,p2,pad);
    }
  }
}

void TreatJIS(FILE *in,FILE *out,int outcode,int notation,char ki[],char ko[],int control,int pad)
{
  int intwobyte = FALSE,temp,p1,p2;
  int data[8];

  while ((p1 = fgetc(in)) != EOF) {
    if (p1 == ESC) {
      temp = fgetc(in);
      SkipESCSeq(in,temp,&intwobyte);
      if ((p1 = fgetc(in)) == EOF)
        exit(1);
    }
    if (intwobyte) {
      p2 = fgetc(in);
      data[SJS1] = data[EUC1] = data[JIS1] = data[KT1] = p1;
      data[SJS2] = data[EUC2] = data[JIS2] = data[KT2] = p2;
      jis2sjis(&data[SJS1],&data[SJS2]);
      data[EUC1] += EUC_OFFSET;
      data[EUC2] += EUC_OFFSET;
      data[KT1] -= KUTEN_OFFSET;
      data[KT2] -= KUTEN_OFFSET;
      print2byte(out,outcode,notation,data,ki,ko,pad);
    }
    else if (!intwobyte && ISASCII(p1))
      print1byte(out,notation,p1,control,pad);
  }
}

void TreatSJIS(FILE *in,FILE *out,int outcode,int notation,char ki[],char ko[],int control,int pad)
{
  int p1,p2;
  int data[8];

  while ((p1 = fgetc(in)) != EOF) {
    if SJIS1(p1) {
      p2 = fgetc(in);
      if SJIS2(p2) {
        data[SJS1] = data[EUC1] = data[JIS1] = data[KT1] = p1;
        data[SJS2] = data[EUC2] = data[JIS2] = data[KT2] = p2;
        sjis2jis(&data[EUC1],&data[EUC2]);
        data[EUC1] += EUC_OFFSET;
        data[EUC2] += EUC_OFFSET;
        sjis2jis(&data[JIS1],&data[JIS2]);
        sjis2jis(&data[KT1],&data[KT2]);
        data[KT1] -= KUTEN_OFFSET;
        data[KT2] -= KUTEN_OFFSET;
        print2byte(out,outcode,notation,data,ki,ko,pad);
      }
      else {
        print1byte(out,notation,p1,control,pad);
        print1byte(out,notation,p2,control,pad);
      }
    }
    else if ISASCII(p1)
      print1byte(out,notation,p1,control,pad);
    else if HANKATA(p1)
      printhwkana(out,outcode,notation,142,p1,pad);
  }
}

void TreatASCII(FILE *in,FILE *out,int notation,int control,int pad)
{
  int p1;

  while ((p1 = fgetc(in)) != EOF)
    if ISASCII(p1)
      print1byte(out,notation,p1,control,pad);
}

int getline(FILE *in,char *myarray)
{
  if (fgets(myarray,100,in) == NULL)
    return 0;
  else
    return 1;
}

int hex2dec(int data)
{
  if (data >= '0' && data <= '9')
    return (data - ASCII_ZERO);
  else if (data >= 'A' && data <= 'F')
    return (data - 55);
}

int convert2code(int p1,int p2,int incode)
{
  if (incode == KUTEN)
    return (((p1 - ASCII_ZERO) * 10) + (p2 - ASCII_ZERO));
  else
    return ((hex2dec(p1) * 16) + hex2dec(p2));
}

void TreatCodes(FILE *in,FILE *out,int outcode,int notation,int pad,char ki[],char ko[])
{
  int y,p1,p2,incode,offset,okay;
  int data[8];
  char myarray[100],temparray[10];

  while (getline(in,myarray)) {
    okay = TRUE;
    offset = 0;
    switch (toup(myarray[0])) {
      case 'K' :
        incode = KUTEN;
        offset = 1;
        break;
      case 'X' :
        incode = EUC;
        offset = 1;
        break;
      default :
        incode = JISORSJIS;
        break;
    }
    for (y = 0;y < 4;y++) {
      if VALID(toup(myarray[y + offset]))
        temparray[y] = toup(myarray[y + offset]);
      else {
        fprintf(stderr,"Invalid character code! Skipping...\n");
        okay = FALSE;
        incode = NOTSET;
        break;
      }
    }
    if (okay) {
      p1 = convert2code(temparray[0],temparray[1],incode);
      p2 = convert2code(temparray[2],temparray[3],incode);
      if (incode == JISORSJIS && ISJIS(p1) && ISJIS(p2))
        ;
      else if (incode == JISORSJIS && SJIS1(p1) && SJIS2(p2))
        sjis2jis(&p1,&p2);
      else if (incode == EUC && ISEUC(p1) && ISEUC(p2)) {
        p1 -= EUC_OFFSET;
        p2 -= EUC_OFFSET;
      }
      else if (incode == KUTEN && ISKUTEN(p1) && ISKUTEN(p2)) {
        p1 += KUTEN_OFFSET;
        p2 += KUTEN_OFFSET;
      }
      else {
        fprintf(stderr,"Invalid character code! Skipping...\n");
        okay = FALSE;
      }
    }
    if (okay) {
      data[SJS1] = data[EUC1] = data[JIS1] = data[KT1] = p1;
      data[SJS2] = data[EUC2] = data[JIS2] = data[KT2] = p2;
      jis2sjis(&data[SJS1],&data[SJS2]);
      data[EUC1] += EUC_OFFSET;
      data[EUC2] += EUC_OFFSET;
      data[KT1] -= KUTEN_OFFSET;
      data[KT2] -= KUTEN_OFFSET;
      print2byte(out,outcode,notation,data,ki,ko,pad);
    }
  }
}

void TreatOneCode(char mydata[])
{
  int y,p1,p2,incode,offset = 0,okay = TRUE;
  int data[8];
  char temparray[10];

  switch (toup(mydata[0])) {
    case 'K' :
      incode = KUTEN;
      offset = 1;
      break;
    case 'X' :
      incode = EUC;
      offset = 1;
      break;
    default :
      incode = JISORSJIS;
      break;
  }
  for (y = 0;y < 4;y++) {
    if VALID(toup(mydata[y + offset]))
      temparray[y] = toup(mydata[y + offset]);
    else {
      fprintf(stderr,"Invalid character code! Exiting...\n");
      exit(1);
    }
  }
  if (okay) {
    p1 = convert2code(temparray[0],temparray[1],incode);
    p2 = convert2code(temparray[2],temparray[3],incode);
    if (incode == JISORSJIS && ISJIS(p1) && ISJIS(p2))
      fprintf(stderr,"NOTE: Input code was detected as JIS\n\n");
    else if (incode == JISORSJIS && SJIS1(p1) && SJIS2(p2)) {
      fprintf(stderr,"NOTE: Input code was detected as Shift-JIS\n\n");
      sjis2jis(&p1,&p2);
    }
    else if (incode == EUC && ISEUC(p1) && ISEUC(p2)) {
      fprintf(stderr,"NOTE: Input code was detected as EUC\n\n");
      p1 -= EUC_OFFSET;
      p2 -= EUC_OFFSET;
    }
    else if (incode == KUTEN && ISKUTEN(p1) && ISKUTEN(p2)) {
      fprintf(stderr,"NOTE: Input code was detected as KUTEN\n\n");
      p1 += KUTEN_OFFSET;
      p2 += KUTEN_OFFSET;
    }
    else {
      fprintf(stderr,"Invalid character code! Exiting...\n");
      exit(1);
    }
  }
  if (okay) {
    data[SJS1] = data[EUC1] = data[JIS1] = data[KT1] = p1;
    data[SJS2] = data[EUC2] = data[JIS2] = data[KT2] = p2;
    jis2sjis(&data[SJS1],&data[SJS2]);
    data[EUC1] += EUC_OFFSET;
    data[EUC2] += EUC_OFFSET;
    data[KT1] -= KUTEN_OFFSET;
    data[KT2] -= KUTEN_OFFSET;
    fprintf(stderr,"Shift-JIS:  %02X-%02X\n",data[SJS1],data[SJS2]);
    fprintf(stderr,"EUC:        %02X-%02X\n",data[EUC1],data[EUC2]);
    fprintf(stderr,"JIS:        %02X-%02X\n",data[JIS1],data[JIS2]);
    fprintf(stderr,"ASCII       %c%c\n",data[JIS1],data[JIS2]);
    fprintf(stderr,"KUTEN:      %02d-%02d\n",data[KT1],data[KT2]);
  }
}
