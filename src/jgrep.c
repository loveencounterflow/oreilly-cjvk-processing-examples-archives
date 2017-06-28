/*
Program: JGREP.C
Version: 1.0b1
Date:    January 1, 1992
Author:  Ken R. Lunde, Adobe Systems Incorporated
  EMAIL: lunde@adobe.com
  MAIL : 1585 Charleston Road, P.O. Box 7900, Mountain View, CA 94039-7900
Type:    A simplified GREP-like utility which recognizes two-byte character
         sequences in Japanese Shift-JIS or EUC code.
Code:    ANSI C (portable)

PORTABILITY:
This source code was written so that it would be portable on C compilers which
conform to the ANSI C standard.

There are 4 lines which have been commented out. These lines of code are
necessary to develop a program which handles command-line arguments on a
Macintosh. I left these lines in so that it would be easier to enhance/debug
the program at a later stage. For those of you who wish to compile this program
for use on a Macintosh, simply delete the comments from those 4 lines of code,
add the ANSI library to the THINK C project, and then build the application.
You then have a double-clickable application, which when launched, will greet
you with a Macintosh-style user interface. Note that the resulting file's
creator is "????," and is type TEXT. This means that double-clicking the file
will not open it (unless you are running an application substitution program
like HandOffII), but you should first launch the text editor then open the
output file by choosing OPEN from the FILE menu.

Portions of this program are copyright Symantec Corporation as I used Think C
version 5.0.1 as the development platform (THINK C now has 100% ANSI C
compatibility!!!).

DISTRIBUTION AND RESTRICTIONS ON USAGE:
 1) Please give this source code away to your friends at no charge.
 2) Please try to compile this source code on various platforms to check for
    portablity, and please report back to me with any results be they good or
    bad. Suggestions are always welcome.
 3) Only use this program on a copy of a file -- do not use an original. This
    is just common sense.
 4) This source code or a compiled version may be bundled with commercial
    software as long as the author is notified beforehand. The author's name
    should also be mentioned in the credits.
 5) Feel free to use any of the algorithms for your own work. Many of them are
    being used in other programs I have written.
 6) The most current version can be obtained through FTP at ucdavis.edu
    (128.120.2.1) in the pub/JIS/C directory, or by requesting a copy
    directly from me.

DESCRIPTION:
 1) Supports only Shift-JIS and EUC codes. New- and Old-JIS support may be
    added at a later time.
 2) The number of lines which matched the pattern is displayed to the screen
    (through stderr), not printed to the file.

OPERATION:
 1) The UNIX-style command-line is

    jgrep [options] [infile] [outfile]

    Note that [infile] and [outfile] can be replaced by redirecting stdin/
    stdout on UNIX systems.
 2) The first optional flag is "-n," and this will print the line number of the
    input file from which the line was taken.
 3) The second optional flag is "-x," and does an inverse pattern match, namely
    that all lines which do NOT match the pattern will be output.
 4) The [infile] field is optional as one can redirect stdin.
 5) The [outfile] field is also optional. If no [outfile] field is specified,
    the program will semi-intelligently change the file's name. The program
    simply scans the [outfile] field, finds the last period in it, terminates
    the string at that point, and tacks on ".out" ("output") as an
    extension. Here are some example command lines, and the resulting outfile
    names:

    a) jgrep sig.jpn                     = sig.out
    b) jgrep sig.jpn.txt                 = sig.jpn.out
    c) jgrep sig                         = sig.out

    This is very useful for MS-DOS users since a filename such as sig.jpn.out
    will not result in converting a file called sig.jpn.

    Also note that if the outfile and infile have the same name, the program
    will not work, and data will be lost. I tried to build safe-guards against
    this. For example, note how my program will change the outfile name so
    that it does not overwrite the infile:
    
    a) jgrep sig.out                     = sig-.out
    b) jgrep sig.jpn sig.jpn             = sig-.jpn
    c) jgrep sig-.out                    = sig--.out
    
    If only the [infile] is given, a hyphen is inserted after the last period,
    and the extension is then reattached. If the outfile is specified by the
    user, then it will search for the last period (if any), attach a hyphen,
    and finally attach the proper extension). This sort of protection is NOT
    available from this program if stdin/stdout are used.

ADDITIONAL NOTES:
I spent quite a long time trying to locate a program for the Macintosh which
performs the same function as "grep" on UNIX systems, namely to output specific
lines, which match a specified pattern, from an input file -- an extracting of
lines. I finally found source code for a program which does this on page 117
and 165 of "The C Programming Language" (second edition) by Kernighan and
Ritchie, 1988, Prentice Hall. What I simply did was to port this small program
to the Macintosh while still retaining the UNIX-style command-line argument
feature. I also used a modified version of the function found on page 279 of
"Algorithms in C" by Sedgewick, 1990, Addison-Wesley. The major modifications
were to make the program work with 2-byte encoded Japanese text in either EUC
or SHIFT-JIS code.

As many may know, "grep" is a UNIX function which stands for "global regular
expression print" (how about Gregior, Ritchie, Ebersole, and Pike -- the names
of the original authors?), and is used to print lines from files based on
pattern matching. This particular version of grep only has a few features, and
I plan to add more in the near future.

I used the "ccommand" function described on pages 122-124 of the Think C
Standard Libraries Reference. This function handles the command-line argument
handling, and displays the window and dialog. It is quite useful for running
UNIX-based programs on the Macintosh. Use it as an example for your own
programs.
*/

/* #include <console.h>
#include <stdlib.h> */
#include <stdio.h>
#include <string.h>

#define MAXLINLEN   1000
#define MAXPATLEN   100
#define PERIOD      '.'
#define FALSE       0
#define TRUE        1
#define ISSJIS1(A)  (((A >= 129) && (A <= 159)) || ((A >= 224) && (A <= 239)))
#define ISSJIS2(A)  ((A >= 64) && (A <= 252))
#define ISEUC(A)    ((A >= 161) && (A <= 254))

/* int ccommand(char ***p); */
int getline(FILE *in,char *line,int max);
int stringsearch(char *pattern,char *string);
int nextchar(char *string,int index);

main(int argc,char **argv)
{
  FILE *in,*out;
  char infilename[100],outfilename[100],extension[5];
  char line[MAXLINLEN],pattern[MAXPATLEN];
  long lineno = 0;
  int c,except = FALSE,number = FALSE,found = 0;

/*  argc = ccommand(&argv); */

  while ((--argc > 0 ) && ((*++argv)[0] == '-')) {
    while (c = *++argv[0]) {
      switch (c) {
        case 'x' :
          except = TRUE;
          break;
        case 'n' :
          number = TRUE;
          break;
        default :
          fprintf(stderr,"jgrep: illegal option %c\n",c);
          exit(1);
      }
    }
  }
  if (argc == 0) {
    fprintf(stderr,"Usage: jgrep [-x] [-n] pattern [infile] [outfile]\n");
    exit(1);
  }
  else {
    strcpy(pattern,*argv++);
    argc--;
  }
  if (argc == 0) {
    in = stdin;
    out = stdout;
  }
  else if (argc > 0) {
    strcpy(extension,".out");
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
    }
    else if (argc > 1) {
      strcpy(infilename,*argv);
      strcpy(outfilename,*++argv);
      if (!strcmp(infilename,outfilename)) {
        if (strchr(outfilename,PERIOD) != NULL)
          *strrchr(outfilename,PERIOD) = '\0';
        strcat(outfilename,"-");
        strcat(outfilename,extension);
      }
    }
    if ((in = fopen(infilename,"r")) == NULL) {
      fprintf(stderr,"\nCannot open %s\n",infilename);
      exit(1);
    }
    if ((out = fopen(outfilename,"w")) == NULL) {
      fprintf(stderr,"\nCannot open %s\n",outfilename);
      exit(1);
    }
  }
  while (getline(in,line,MAXLINLEN) > 0) {
    lineno++;
    if ((stringsearch(pattern,line) != 0) != except) { /* replaced strstr() */
      if (number) {
        fprintf(out,"%ld:",lineno);
      }
      fprintf(out,"%s",line);
      found++;
    }
  }
  if (except)
    fprintf(stderr,"\nLines not matching pattern [%s]: %d\n",pattern,found);
  else
    fprintf(stderr,"\nLines matching pattern [%s]: %d\n",pattern,found);
  exit(0);
}


int getline(FILE *in,char *line,int max)
{
  if (fgets(line,max,in) == NULL)
    return 0;
  else
    return 1;
}


int stringsearch(char *p,char *a) /* Fixed by Michael Henning. Thanks, Michael! */
{
  int i,j,M = strlen(p),N = strlen(a);
  int limit;

  limit = N - M + 1;
  for (i = 0; i < limit; i += nextchar(a,i)) {
    for (j = 0; j < M; j++) {
      if (a[i + j] != p[j])
        break;
    }
    if (j == M)
	return 1;
  }
  return 0;
}


int nextchar(char *s,int i) /* my home brew to handle 2-byte Japanese codes */
{
  unsigned char *p;
  p = (unsigned char *) s;
  if ((ISSJIS1(p[i]) || ISEUC(p[i])) && (ISSJIS2(p[i + 1]) || ISEUC(p[i + 1])))
    return 2;
  else
    return 1;
}
