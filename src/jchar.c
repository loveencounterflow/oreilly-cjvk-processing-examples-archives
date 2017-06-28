/*
Program: jchar.c
Version: 3.0
Date:    July 1, 1993
Author:  Ken R. Lunde, Adobe Systems Incorporated
  EMAIL: lunde@mv.us.adobe.com
  MAIL : 1585 Charleston Road, P.O. Box 7900, Mountain View, CA 94039-7900
Type:    A tool for generating various Japanese character sets.
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
 3) This source code or a compiled version may be bundled with commercial
    software as long as the author is notified beforehand. The author's name
    should also be mentioned in the credits.
 4) Feel free to use any of the algorithms for your own work. Many of them are
    being used in other tools I have written.
 5) The most current version can be obtained by requesting a copy directly
    from me.

DESCRIPTION:
 1) Supports Shift-JIS, EUC, New-JIS, Old-JIS, and NEC-JIS codes for output.
 2) This tool generates half-width katakana in both Shift-JIS and EUC codes.
 3) The printable ASCII character set, the 1945 Joyo Kanji, the 284
    Jinmei-yo Kanji, and the 1006 Gakushu Kanji can be generated.
 4) In the case of JIS X 0208-1990, the row number is given before the row.
 5) In the case of Gakushu Kanji, the list is separated into the 6 grade
    levels.
 6) One can choose whether to have the text wrap, and at how many columns.
*/

#ifdef THINK_C
#include <console.h>
#include <stdlib.h>
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define NOTSET           0
#define FALSE            0
#define TRUE             1
#define NEW              1
#define OLD              2
#define NEC              3
#define EUC              4
#define SJIS             5
#define ASCII            6
#define KUTEN_OFFSET     32
#define ASCII_ZERO       48
#define JIS_START        33
#define JIS_END          126
#define ASCII_START      33
#define ASCII_END        126
#define HW_KANA_START    161
#define HW_KANA_END      223
#define EUC_START        161
#define EUC_END          254
#define JOYO_TOTAL       1945
#define JINMEIYO_TOTAL   284
#define GAKUSHU_TOTAL    1006
#define DEFAULT_MAX      78
#define FIRSTBYTE(val)   (((val) / 100) + KUTEN_OFFSET)
#define SECONDBYTE(val)  (((val) % 100) + KUTEN_OFFSET)
#define ARRAYSIZE(array) (sizeof(array) / sizeof(array[0]))
/* The following 3 lines of code are used to establish the default output codes
 * when using the "-o[CODE]" option. They are self-explanatory, and easy to
 * change.
 */
#define DEFAULT          SJIS     /* default output code */
#define DEFAULT_KI       ""       /* default kanji-in code */
#define DEFAULT_KO       ""       /* default kanji-out code */

void printASCII(FILE *out,int wrap,int max,int suppress);
void printJIS(FILE *out,int outcode,int wrap,int max,char ki[],char ko[],int suppress);
void printEUC(FILE *out,int wrap,int max,int suppress);
void printSJIS(FILE *out,int wrap,int max,int suppress);
void printJOYO(FILE *out,int outcode,int wrap,int max,char ki[],char ko[],int suppress);
void printJINMEI(FILE *out,int outcode,int wrap,int max,char ki[],char ko[],int suppress);
void printGAKUSHU(FILE *out,int outcode,int wrap,int max,char ki[],char ko[],int suppress);
void jis2sjis(int *p1,int *p2);
void dohelp(char toolname[]);
int toup(int data);
int getoutcode(int data,char ki[],char ko[]);
#ifdef THINK_C
int ccommand(char ***p);
#endif

void main(int argc,char **argv)
{
  FILE *out;
  int y,max,suppress = FALSE;
  int makejinmei = FALSE,makejis = FALSE,makeascii = FALSE,makejoyo = FALSE;
  int usersetcol = FALSE,wrap = FALSE,outcode = NOTSET,select = FALSE,makegakushu = FALSE;
  char outfilename[100],ki[10],ko[10],toolname[100];

#ifdef THINK_C
  argc = ccommand(&argv);
#endif

  strcpy(toolname,*argv);
  while (--argc > 0 && (*++argv)[0] == '-')
    switch (toup(*++argv[0])) {
      case 'A' :
        makeascii = select = TRUE;
        break;
      case 'G' :
        makegakushu = select = TRUE;
        break;
      case 'H' :
        dohelp(toolname);
        break;
      case 'J' :
        makejoyo = select = TRUE;
        break;
      case 'K' :
        makejis = select = TRUE;
        break;
      case 'O' :
        outcode = getoutcode(toup(*++argv[0]),ki,ko);
        break;
      case 'P' :
        makejinmei = select = TRUE;
        break;
      case 'S' :
        suppress = TRUE;
        break;
      case 'W' :
        max = y = 0;
        wrap = TRUE;
        if (*++argv[0] != '\0') {
          usersetcol = TRUE;
          while (argv[0][y] != '\0' && argv[0][y] >= '0' && argv[0][y] <= '9')
            max = (max * 10) + (argv[0][y++] - ASCII_ZERO);
        }
        break;
      default :
        fprintf(stderr,"Illegal option \"-%c\"! Try using the \"-h\" option for help.\n",*argv[0]);
        fprintf(stderr,"Usage: %s [-options] [outfile]\nExiting...\n",toolname);
        exit(1);
        break;
    }
  if (outcode == NOTSET) {
    strcpy(ki,DEFAULT_KI);
    strcpy(ko,DEFAULT_KO);
    outcode = DEFAULT;
  }
  if (!select)
    makejis = TRUE;
  if (wrap && !usersetcol)
    max = DEFAULT_MAX;
  if (argc == 0)
    out = stdout;
  else if (argc > 0) {
    strcpy(outfilename,*argv);
    if ((out = fopen(outfilename,"w")) == NULL) {
      fprintf(stderr,"Cannot open %s! Exiting...\n",outfilename);
      exit(1);
    }
  }
  if (makeascii)
    printASCII(out,wrap,max,suppress);
  if (makejinmei)
    printJINMEI(out,outcode,wrap,max,ki,ko,suppress);
  if (makejoyo)
    printJOYO(out,outcode,wrap,max,ki,ko,suppress);
  if (makegakushu)
    printGAKUSHU(out,outcode,wrap,max,ki,ko,suppress);
  if (makejis) {
    if (!suppress)
      fprintf(out,"JIS X 0208-1990\n\n");
    switch (outcode) {
      case NEW :
      case OLD :
      case NEC :
        printJIS(out,outcode,wrap,max,ki,ko,suppress);
        break;
      case EUC :
        printEUC(out,wrap,max,suppress);
        break;
      case SJIS :
        printSJIS(out,wrap,max,suppress);
        break;
    }
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

int getoutcode(int data,char ki[],char ko[])
{
  if (data == 'E')
    return EUC;
  else if (data == 'S')
    return SJIS;
  else if (data == 'J') {
    strcpy(ki,"\033$B");
    strcpy(ko,"\033(J");
    return NEW;
  }
  else if (data == 'O') {
    strcpy(ki,"\033$@");
    strcpy(ko,"\033(J");
    return OLD;
  }
  else if (data == 'N') {
    strcpy(ki,"\033K");
    strcpy(ko,"\033H");
    return NEC;
  }
  else
    return NOTSET;
}

void dohelp(char toolname[])
{
  printf("** %s v3.0 (July 1, 1993) **\n\n",toolname);
  printf("Written by Ken R. Lunde, Adobe Systems Incorporated\nlunde@mv.us.adobe.com\n\n");
  printf("Usage: %s [-options] [outfile]\n\n",toolname);
  printf("Tool description: This tool is a utility for generating various Japanese\n");
  printf("character sets in any code. This includes all the characters specified in\n");
  printf("JIS X 0208-1990, half-width katakana (EUC and Shift-JIS output only), the\n");
  printf("94 printable ASCII/JIS-Roman characters, the 1945 Joyo Kanji, the 284\n");
  printf("Jinmei-yo Kanji, and the 1006 Gakushu Kanji.\n\n");
  printf("Options include:\n\n");
  printf("  -a        Builds an ASCII/JIS-Roman list (printable characters only)\n");
  printf("  -g        Builds the Gakushu Kanji list\n");
  printf("  -h        Displays this help page, then exits\n");
  printf("  -j        Builds the Joyo Kanji list\n");
  printf("  -k        Builds the JIS X 0208-1990 list\n");
  printf("  -o[CODE]  Builds lists in CODE format (default is Shift-JIS if this option\n");
  printf("            is not specified, if CODE is not specified, or if CODE is invalid)\n");
  printf("  -p        Builds the Jinmei-yo Kanji list\n");
  printf("  -s        Suppresses headers and row number information\n");
  printf("  -w[NUM]   Wraps output lines at NUM columns (if NUM is not specified, 78\n");
  printf("            is used as the default value)\n\n");
  printf("NOTE: CODE has five possible values: \"e\" = EUC; \"s\" = Shift-JIS;\n");
  printf("      \"j\" = New-JIS; \"o\" = Old-JIS; and \"n\" = NEC-JIS\n\n");
  exit(0);
}

void printASCII(FILE *out,int wrap,int max,int suppress)
{
  int i,column = 0;

  if (!suppress)
    fprintf(out,"ASCII/JIS-Roman\n\n");
  for (i = ASCII_START;i <= ASCII_END;fprintf(out,"%c",i),column++,i++)
    if (wrap)
      if (column >= max) {
        fprintf(out,"\n");
        column = 0;
      }
  fprintf(out,"\n");
}

void printJIS(FILE *out,int outcode,int wrap,int max,char ki[],char ko[],int suppress)
{
  int i,j,column = 0;

  for (i = JIS_START;i <= JIS_END;i++) {
    if (i == JIS_START && !suppress)
      fprintf(out,"Row number %1d:\n",(i - KUTEN_OFFSET));
    else if (i > JIS_START && !suppress)
      fprintf(out,"\nRow number %1d:\n",(i - KUTEN_OFFSET));
    if (!suppress || i == JIS_START)
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          fprintf(out,"%s",ki);
          break;
        default :
          break;
      }
    for (j = JIS_START;j <= JIS_END;fprintf(out,"%c%c",i,j),column += 2,j++) {
      if (wrap)
        if (column >= max) {
          switch (outcode) {
            case NEW :
            case OLD :
            case NEC :
              fprintf(out,"%s\n%s",ko,ki);
              break;
            default :
              break;
          }
          column = 0;
        }
    }
    if (!suppress) {
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
         fprintf(out,"%s",ko);
         break;
        default :
         break;
      }
      column = 0;
    }
  }
  if (suppress)
    fprintf(out,"%s\n",ko);
  else
    fprintf(out,"\n");
}

void printEUC(FILE *out,int wrap,int max,int suppress)
{
  int i,j,column = 0;

  for (i = EUC_START;i <= EUC_END;i++) {
    if (i > EUC_START && !suppress)
      fprintf(out,"\n");
    if (!suppress)
      fprintf(out,"Row number %1d:\n",(i - 160));
    for (j = EUC_START;j <= EUC_END;fprintf(out,"%c%c",i,j),column += 2,j++)
      if (wrap)
        if (column >= max) {
          fprintf(out,"\n");
          column = 0;
        }
    if (!suppress)
      column = 0;
  }
  column = 0;
  fprintf(out,"\n");
  if (!suppress)
    fprintf(out,"Half-width katakana:\n");
  for (i = HW_KANA_START;i <= HW_KANA_END;fprintf(out,"%c%c",142,i),column++,i++)
    if (wrap)
      if (column >= max) {
        fprintf(out,"\n");
        column = 0;
      }
  fprintf(out,"\n");
}

void printSJIS(FILE *out,int wrap,int max,int suppress)
{
  int i,j = 64,count = 1,column = 0;

  if (!suppress)
    fprintf(out,"Row number %1d:\n",count);
  for (i = 129;i <= 159;i++) {
    for (j = 64;j <= 252;j++) {
      if (j != 127) {
        fprintf(out,"%c%c",i,j);
        column += 2;
        if (wrap)
          if (column >= max) {
            fprintf(out,"\n");
            column = 0;
          }
      }
      if (j == 158 || j == 252) {
        count++;
        if (!suppress) {
          column = 0;
          fprintf(out,"\nRow number %1d:\n",count);
        }
      }
    }
    if (!suppress)
      column = 0;
  }
  j = 64;
  for (i = 224;i <= 239;i++) {
    for (j = 64;j <= 252;j++) {
      if (j != 127) {
        fprintf(out,"%c%c",i,j);
        column += 2;
        if (wrap)
          if (column >= max) {
            fprintf(out,"\n");
            column = 0;
          }
      }
      if (j == 158 || j == 252) {
        count++;
        if (count <= 94 && !suppress) {
          column = 0;
          fprintf(out,"\nRow number %1d:\n",count);
        }
      }
    }
    if (!suppress)
      column = 0;
  }
  column = 0;
  fprintf(out,"\n");
  if (!suppress)
    fprintf(out,"Half-width katakana:\n");
  for (i = HW_KANA_START;i <= HW_KANA_END;fprintf(out,"%c",i),column++,i++)
    if (wrap)
      if (column >= max) {
        fprintf(out,"\n");
        column = 0;
      }
  fprintf(out,"\n");
}

void printJINMEI(FILE *out,int outcode,int wrap,int max,char ki[],char ko[],int suppress)
{
  int p1,p2,i,column = 0;
  static unsigned short pk[] = {
    1604,1610,1611,1615,1616,1620,1628,1629,1630,1641,
    1643,1652,1671,1674,1675,1684,1693,1712,1715,1720,
    1735,1745,1780,1781,1787,1802,1832,1837,1856,1866,
    1901,1914,1930,1980,1982,1989,1993,2006,2048,2064,
    2076,2082,2103,2109,2121,2135,2139,2144,2192,2209,
    2212,2238,2245,2251,2253,2254,2260,2262,2274,2275,
    2280,2307,2310,2322,2329,2337,2343,2393,2430,2453,
    2455,2464,2467,2472,2474,2481,2486,2508,2516,2523,
    2524,2532,2541,2567,2623,2627,2628,2632,2640,2651,
    2667,2683,2691,2709,2724,2804,2812,2814,2815,2837,
    2851,2907,2952,2955,2956,2957,2963,2970,2976,2977,
    2990,3017,3025,3027,3031,3052,3054,3071,3087,3124,
    3126,3133,3160,3173,3180,3183,3187,3257,3354,3358,
    3378,3379,3383,3433,3467,3468,3475,3479,3486,3494,
    3504,3507,3522,3541,3550,3586,3610,3619,3639,3648,
    3653,3656,3661,3665,3680,3685,3687,3746,3803,3820,
    3823,3850,3851,3855,3856,3864,3865,3868,3872,3879,
    3890,3921,3923,3935,3975,4005,4023,4027,4069,4076,
    4093,4102,4107,4118,4123,4143,4171,4186,4189,4243,
    4267,4269,4294,4308,4317,4318,4351,4374,4379,4382,
    4391,4406,4413,4426,4450,4473,4474,4477,4479,4487,
    4504,4508,4514,4515,4520,4524,4544,4552,4554,4558,
    4582,4585,4586,4591,4592,4594,4628,4631,4638,4639,
    4642,4643,4654,4659,4660,4666,4670,4671,4672,4701,
    4704,4729,4733,4743,4850,5006,5287,5437,5534,5869,
    5871,5879,5880,5886,5957,6031,6160,6211,6213,6270,
    6291,6326,6389,6402,6486,6611,6640,6789,6926,6937,
    7038,7091,7193,7229,7233,7546,7557,7666,7776,8083,
    8105,8353,8405,8406 };

  if (JINMEIYO_TOTAL != ARRAYSIZE(pk)) {
    fprintf(stderr,"Incomplete data structure for Jinmei-yo Kanji list! Exiting...\n");
    exit(1);
  }
  if (!suppress) {
    fprintf(out,"MACHINE-READABLE JINMEI-YO KANJI LIST (in JIS order)\n");
    fprintf(out,"NOTE: the last two kanji are unique to JIS X 0208-1990, and thus\n");
    fprintf(out,"      may not display properly on all systems\n\n");
  }
  for (i = 0;i < JINMEIYO_TOTAL;i++) {
    if (column == 0)
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          fprintf(out,"%s",ki);
          break;
        default :
          break;
      }
    if (wrap && column >= max) {
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          fprintf(out,"%s\n%s",ko,ki);
          break;
        case EUC :
        case SJIS :
          fprintf(out,"\n");
          break;
        default :
          break;
      }
      column = 0;
    }
    switch (outcode) {
      case NEW :
      case OLD :
      case NEC :
        fprintf(out,"%c%c",FIRSTBYTE(pk[i]), SECONDBYTE(pk[i]));
        column += 2;
        break;
      case EUC :
        fprintf(out,"%c%c",FIRSTBYTE(pk[i]) + 128, SECONDBYTE(pk[i]) + 128);
        column += 2;
        break;
      case SJIS :
        p1 = FIRSTBYTE(pk[i]);
        p2 = SECONDBYTE(pk[i]);
        jis2sjis(&p1,&p2);
        fprintf(out,"%c%c",p1,p2);
        column += 2;
        break;
      default :
        break;
    }
  }
  switch (outcode) {
    case NEW :
    case OLD :
    case NEC :
      fprintf(out,"%s\n",ko);
      break;
    case EUC :
    case SJIS :
      fprintf(out,"\n");
      break;
    default :
      break;
  }
}

void printGAKUSHU(FILE *out,int outcode,int wrap,int max,char ki[],char ko[],int suppress)
{
  int p1,p2,i,column = 0;
  static unsigned short gk[] = {
    1676,1706,1711,1763,1806,1827,1828,1848,1854,1913,
    1956,2104,2157,2244,2266,2269,2285,2378,2404,2411,
    2462,2493,2527,2624,2716,2719,2745,2750,2769,2790,
    2810,2823,2854,2874,2929,2948,2987,3014,3069,3125,
    3145,3169,3221,3224,3236,3248,3254,3272,3273,3278,
    3365,3380,3413,3428,3471,3543,3561,3570,3578,3614,
    3723,3736,3758,3883,3892,3894,3915,3982,4012,4120,
    4224,4360,4430,4458,4460,4528,4609,4647,4651,4727,
    1690,1709,1732,1764,1783,1811,1831,1838,1840,1842,
    1846,1872,1881,1883,1904,1908,1916,1949,1958,1972,
    2054,2061,2068,2073,2102,2105,2113,2161,2177,2191,
    2194,2215,2221,2265,2327,2333,2355,2421,2422,2432,
    2437,2445,2465,2469,2476,2482,2487,2488,2509,2513,
    2545,2552,2566,2571,2581,2585,2603,2645,2657,2678,
    2727,2748,2752,2755,2763,2770,2791,2794,2811,2828,
    2850,2869,2883,2909,2921,2953,2981,3015,3076,3107,
    3109,3120,3123,3138,3162,3184,3217,3218,3228,3230,
    3258,3267,3294,3305,3316,3340,3386,3431,3432,3446,
    3470,3511,3546,3547,3551,3567,3575,3611,3625,3627,
    3630,3644,3679,3725,3732,3737,3763,3765,3776,3786,
    3790,3812,3817,3827,3841,3866,3878,3889,3947,3967,
    3968,3994,4030,4054,4167,4187,4212,4225,4238,4266,
    4276,4293,4344,4369,4372,4392,4432,4436,4451,4471,
    4475,4478,4480,4507,4543,4549,4572,4593,4604,4735,
    1613,1634,1637,1649,1653,1669,1673,1687,1691,1701,
    1731,1743,1756,1803,1816,1825,1829,1857,1906,1911,
    1912,2008,2022,2033,2059,2063,2092,2115,2150,2160,
    2162,2169,2170,2173,2178,2222,2240,2241,2242,2268,
    2272,2276,2281,2315,2324,2358,2372,2376,2406,2409,
    2443,2448,2494,2512,2533,2570,2612,2655,2668,2714,
    2737,2740,2747,2756,2764,2777,2785,2786,2793,2801,
    2816,2834,2844,2852,2871,2872,2873,2882,2885,2903,
    2906,2910,2912,2924,2927,2937,2941,2974,2975,2985,
    3001,3006,3028,3035,3047,3072,3102,3128,3129,3131,
    3132,3140,3142,3204,3216,3246,3320,3359,3374,3387,
    3409,3414,3418,3430,3439,3448,3452,3469,3472,3474,
    3526,3527,3544,3569,3576,3577,3590,3602,3620,3641,
    3674,3677,3711,3720,3730,3748,3752,3757,3771,3774,
    3782,3789,3806,3816,3824,3932,3940,3959,3960,4002,
    4010,4015,4031,4036,4065,4073,4094,4101,4114,4125,
    4129,4134,4135,4142,4155,4173,4184,4194,4201,4210,
    4231,4254,4257,4292,4403,4431,4444,4468,4482,4484,
    4493,4513,4519,4523,4529,4545,4546,4551,4553,4559,
    4578,4614,4625,4630,4648,4673,4683,4693,4709,4734,
    1606,1638,1642,1644,1647,1663,1665,1685,1741,1749,
    1786,1791,1815,1835,1844,1861,1863,1874,1894,1903,
    1918,1925,1938,1948,2016,2017,2041,2049,2056,2074,
    2078,2079,2085,2090,2101,2108,2110,2136,2163,2165,
    2167,2175,2183,2189,2205,2206,2208,2232,2243,2317,
    2319,2320,2331,2334,2342,2361,2371,2375,2382,2390,
    2419,2439,2485,2489,2505,2515,2550,2580,2625,2639,
    2658,2664,2682,2694,2701,2705,2706,2718,2722,2726,
    2736,2742,2743,2746,2765,2778,2789,2803,2813,2826,
    2858,2879,2894,2943,2971,2973,3007,3030,3038,3040,
    3042,3048,3061,3062,3114,3135,3214,3222,3237,3242,
    3249,3262,3265,3266,3279,3285,3310,3319,3350,3367,
    3372,3406,3411,3419,3420,3425,3451,3466,3503,3517,
    3554,3571,3589,3591,3618,3667,3668,3676,3710,3721,
    3733,3744,3756,3784,3815,3818,3832,3835,3839,3914,
    3916,3952,3963,3978,4051,4081,4084,4112,4124,4128,
    4152,4153,4160,4191,4220,4228,4244,4249,4253,4256,
    4281,4301,4330,4350,4386,4394,4404,4414,4417,4421,
    4483,4506,4555,4560,4565,4588,4606,4633,4641,4644,
    4656,4664,4665,4667,4668,4682,4702,4711,4723,4731,
    1621,1655,1660,1688,1736,1742,1750,1753,1755,1773,
    1793,1794,1824,1830,1833,1836,1847,1865,1876,1882,
    1887,1942,1946,1959,2009,2020,2023,2067,2080,2083,
    2112,2127,2133,2153,2155,2176,2179,2186,2213,2229,
    2249,2256,2271,2318,2348,2373,2379,2384,2401,2417,
    2426,2429,2434,2436,2446,2478,2490,2492,2529,2544,
    2554,2559,2614,2626,2638,2642,2646,2650,2661,2663,
    2665,2666,2689,2708,2731,2732,2753,2754,2757,2762,
    2781,2784,2787,2808,2817,2833,2843,2853,2888,2904,
    2949,2950,2964,2988,3021,3023,3058,3079,3080,3082,
    3085,3105,3106,3209,3210,3213,3215,3226,3229,3239,
    3251,3253,3260,3263,3268,3269,3312,3336,3339,3377,
    3392,3393,3404,3407,3412,3416,3427,3454,3463,3464,
    3536,3539,3559,3605,3683,3688,3708,3712,3793,3819,
    3828,3833,3840,3904,3919,3929,3943,4029,4039,4040,
    4070,4078,4083,4087,4122,4130,4147,4156,4157,4159,
    4180,4192,4203,4209,4252,4259,4261,4272,4283,4313,
    4329,4339,4341,4419,4420,4434,4442,4502,4530,4534,
    4538,4608,4612,4617,4646,1659,1668,1672,1707,1739,
    1768,1772,1870,1905,1940,1953,1955,1968,1984,2012,
    2019,2039,2042,2077,2088,2089,2114,2131,2159,2201,
    2227,2231,2248,2258,2341,2347,2357,2364,2367,2374,
    2391,2402,2408,2423,2427,2438,2442,2477,2501,2507,
    2536,2540,2561,2563,2579,2582,2592,2604,2629,2634,
    2649,2659,2686,2693,2729,2749,2768,2774,2775,2776,
    2779,2807,2845,2846,2860,2867,2889,2893,2901,2902,
    2916,2930,2936,2944,2947,2967,2972,2980,2984,2992,
    2993,3013,3067,3075,3088,3143,3146,3166,3168,3203,
    3225,3227,3231,3275,3276,3284,3286,3287,3317,3347,
    3353,3356,3364,3375,3385,3401,3402,3424,3426,3480,
    3520,3521,3534,3540,3542,3545,3572,3573,3588,3603,
    3612,3626,3634,3643,3724,3762,3792,3804,3847,3881,
    3893,3907,3928,3930,3941,3948,3950,3956,3957,4041,
    4053,4061,4067,4075,4202,4219,4234,4236,4237,4250,
    4268,4275,4285,4312,4320,4326,4332,4371,4375,4409,
    4433,4447,4485,4505,4525,4536,4563,4566,4580,4581,
    4587,4602,4607,4655,4715,4732 };

  if (GAKUSHU_TOTAL != ARRAYSIZE(gk)) {
    fprintf(stderr,"Incomplete data structure for Gakushu Kanji list! Exiting...\n");
    exit(1);
  }
  if (!suppress)
    fprintf(out,"MACHINE-READABLE GAKUSHU KANJI LIST (by grade level in JIS order)\n\n");
  for (i = 0;i < GAKUSHU_TOTAL;i++) {
    if (!suppress && i == 0)
      fprintf(out,"Grade Level 1 (80 kanji):\n");
    if (!suppress && (i == 80 || i == 240 || i == 440 || i == 640 || i == 825)) {
      column = 0;
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          fprintf(out,"%s\n",ko);
          break;
        case EUC :
        case SJIS :
          fprintf(out,"\n");
          break;
        default :
          break;
      }
      switch (i) {
        case 80 :
          fprintf(out,"Grade Level 2 (160 kanji):\n");
          break;
        case 240 :
          fprintf(out,"Grade Level 3 (200 kanji):\n");
          break;
        case 440 :
          fprintf(out,"Grade Level 4 (200 kanji):\n");
          break;
        case 640 :
          fprintf(out,"Grade Level 5 (185 kanji):\n");
          break;
        case 825 :
          fprintf(out,"Grade Level 6 (181 kanji):\n");
          break;
        default :
          break;
      }
    }
    if (column == 0)
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          fprintf(out,"%s",ki);
          break;
        default :
          break;
      }
    if (wrap && column >= max) {
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          fprintf(out,"%s\n%s",ko,ki);
          break;
        case EUC :
        case SJIS :
          fprintf(out,"\n");
          break;
        default :
          break;
      }
      column = 0;
    }
    switch (outcode) {
      case NEW :
      case OLD :
      case NEC :
        fprintf(out,"%c%c",FIRSTBYTE(gk[i]), SECONDBYTE(gk[i]));
        column += 2;
        break;
      case EUC :
        fprintf(out,"%c%c",FIRSTBYTE(gk[i]) + 128, SECONDBYTE(gk[i]) + 128);
        column += 2;
        break;
      case SJIS :
        p1 = FIRSTBYTE(gk[i]);
        p2 = SECONDBYTE(gk[i]);
        jis2sjis(&p1,&p2);
        fprintf(out,"%c%c",p1,p2);
        column += 2;
        break;
      default :
        break;
    }
  }
  switch (outcode) {
    case NEW :
    case OLD :
    case NEC :
      fprintf(out,"%s\n",ko);
      break;
    case EUC :
    case SJIS :
      fprintf(out,"\n");
      break;
    default :
      break;
  }
}

void printJOYO(FILE *out,int outcode,int wrap,int max,char ki[],char ko[],int suppress)
{
  int p1,p2,i,column = 0;
  static unsigned short jk[] = {
    1601,1605,1606,1613,1614,1621,1623,1634,1637,1638,
    1642,1644,1645,1646,1647,1649,1650,1651,1653,1654,
    1655,1657,1659,1660,1661,1662,1663,1665,1667,1668,
    1669,1670,1672,1673,1676,1677,1679,1680,1682,1685,
    1687,1688,1689,1690,1691,1701,1702,1703,1704,1706,
    1707,1709,1711,1718,1726,1731,1732,1736,1738,1739,
    1741,1742,1743,1749,1750,1751,1752,1753,1754,1755,
    1756,1757,1758,1759,1760,1763,1764,1767,1768,1771,
    1772,1773,1774,1776,1778,1779,1783,1784,1786,1788,
    1790,1791,1792,1793,1794,1801,1803,1804,1805,1806,
    1807,1811,1813,1815,1816,1817,1821,1823,1824,1825,
    1826,1827,1828,1829,1830,1831,1833,1834,1835,1836,
    1838,1839,1840,1841,1842,1843,1844,1845,1846,1847,
    1848,1850,1852,1853,1854,1857,1858,1859,1861,1863,
    1865,1867,1870,1872,1874,1876,1877,1878,1880,1881,
    1882,1883,1884,1885,1887,1888,1889,1891,1892,1893,
    1894,1903,1904,1905,1906,1907,1908,1911,1912,1913,
    1915,1916,1918,1920,1921,1922,1925,1926,1932,1937,
    1938,1940,1942,1943,1944,1945,1946,1947,1948,1949,
    1951,1952,1953,1954,1955,1956,1957,1958,1959,1961,
    1967,1968,1969,1971,1972,1973,1974,1976,1977,1978,
    1984,2002,2005,2007,2008,2009,2010,2011,2012,2013,
    2014,2016,2017,2018,2019,2020,2021,2022,2023,2024,
    2025,2026,2029,2030,2031,2032,2033,2036,2037,2038,
    2039,2041,2042,2043,2044,2046,2047,2049,2051,2052,
    2053,2054,2055,2056,2057,2059,2061,2062,2063,2067,
    2068,2072,2073,2074,2075,2077,2078,2079,2080,2081,
    2083,2084,2085,2086,2087,2088,2089,2090,2091,2092,
    2093,2094,2101,2102,2104,2105,2107,2108,2110,2112,
    2113,2114,2115,2116,2117,2118,2119,2120,2122,2123,
    2125,2126,2127,2128,2129,2130,2131,2133,2136,2138,
    2140,2142,2145,2149,2150,2151,2152,2153,2154,2155,
    2157,2158,2159,2160,2161,2162,2163,2164,2165,2167,
    2169,2170,2171,2173,2174,2175,2176,2177,2178,2179,
    2180,2181,2182,2183,2185,2186,2187,2189,2191,2193,
    2194,2201,2205,2206,2207,2208,2211,2213,2214,2215,
    2218,2219,2220,2221,2222,2223,2224,2225,2226,2227,
    2228,2229,2231,2232,2233,2235,2236,2237,2239,2240,
    2241,2242,2243,2244,2248,2249,2252,2255,2256,2258,
    2259,2261,2263,2264,2265,2266,2267,2268,2269,2271,
    2272,2276,2278,2281,2282,2283,2285,2286,2288,2289,
    2294,2301,2304,2311,2312,2314,2315,2316,2317,2318,
    2319,2320,2324,2325,2326,2327,2328,2331,2332,2333,
    2334,2335,2336,2338,2339,2340,2341,2342,2344,2347,
    2348,2349,2352,2354,2355,2357,2358,2360,2361,2362,
    2363,2364,2366,2367,2370,2371,2372,2373,2374,2375,
    2376,2378,2379,2380,2382,2383,2384,2385,2387,2388,
    2389,2390,2391,2392,2401,2402,2404,2405,2406,2408,
    2409,2410,2411,2412,2413,2414,2415,2417,2418,2419,
    2421,2422,2423,2424,2425,2426,2427,2428,2429,2432,
    2434,2436,2437,2438,2439,2441,2442,2443,2444,2445,
    2446,2447,2448,2456,2459,2460,2461,2462,2463,2465,
    2466,2468,2469,2470,2471,2475,2476,2477,2478,2482,
    2484,2485,2487,2488,2489,2490,2492,2493,2494,2501,
    2503,2505,2506,2507,2509,2510,2512,2513,2515,2517,
    2518,2519,2520,2521,2522,2525,2527,2529,2530,2531,
    2533,2534,2535,2536,2537,2538,2540,2542,2543,2544,
    2545,2546,2550,2551,2552,2553,2554,2555,2556,2557,
    2558,2559,2561,2563,2564,2565,2566,2568,2570,2571,
    2573,2575,2578,2579,2580,2581,2582,2583,2585,2586,
    2588,2592,2594,2603,2604,2606,2607,2608,2609,2611,
    2612,2614,2616,2618,2620,2622,2624,2625,2626,2629,
    2630,2631,2634,2636,2637,2638,2639,2642,2643,2644,
    2645,2646,2647,2648,2649,2650,2653,2655,2656,2657,
    2658,2659,2660,2661,2662,2663,2664,2665,2666,2668,
    2673,2674,2678,2679,2681,2682,2686,2687,2688,2689,
    2693,2694,2701,2703,2704,2705,2706,2708,2714,2716,
    2717,2718,2719,2720,2722,2723,2726,2727,2729,2731,
    2732,2735,2736,2737,2739,2740,2741,2742,2743,2744,
    2745,2746,2747,2748,2749,2750,2752,2753,2754,2755,
    2756,2757,2760,2761,2762,2763,2764,2765,2767,2768,
    2769,2770,2771,2772,2773,2774,2775,2776,2777,2778,
    2779,2780,2781,2782,2783,2784,2785,2786,2787,2788,
    2789,2790,2791,2792,2793,2794,2801,2802,2803,2805,
    2807,2808,2810,2811,2813,2816,2817,2820,2823,2825,
    2826,2828,2830,2831,2832,2833,2834,2839,2843,2844,
    2845,2846,2847,2848,2849,2850,2852,2853,2854,2855,
    2856,2857,2858,2859,2860,2863,2864,2865,2867,2868,
    2869,2871,2872,2873,2874,2875,2876,2877,2878,2879,
    2881,2882,2883,2884,2885,2887,2888,2889,2891,2892,
    2893,2894,2901,2902,2903,2904,2905,2906,2908,2909,
    2910,2912,2913,2914,2916,2917,2921,2923,2924,2925,
    2927,2928,2929,2930,2932,2933,2934,2935,2936,2937,
    2938,2939,2941,2942,2943,2944,2945,2946,2947,2948,
    2949,2950,2951,2953,2954,2958,2959,2960,2962,2964,
    2965,2966,2967,2968,2969,2971,2972,2973,2974,2975,
    2978,2979,2980,2981,2984,2985,2986,2987,2988,2989,
    2992,2993,2994,3001,3002,3003,3004,3006,3007,3009,
    3012,3013,3014,3015,3016,3018,3020,3021,3022,3023,
    3024,3026,3028,3029,3030,3034,3035,3036,3038,3039,
    3040,3041,3042,3043,3044,3045,3046,3047,3048,3049,
    3050,3051,3055,3057,3058,3059,3060,3061,3062,3066,
    3067,3069,3070,3072,3073,3074,3075,3076,3077,3078,
    3079,3080,3082,3084,3085,3086,3088,3089,3090,3091,
    3092,3094,3102,3103,3105,3106,3107,3108,3109,3111,
    3113,3114,3115,3116,3117,3118,3119,3120,3121,3122,
    3123,3125,3127,3128,3129,3131,3132,3134,3135,3137,
    3138,3139,3140,3141,3142,3143,3144,3145,3146,3147,
    3150,3151,3152,3155,3156,3161,3162,3165,3166,3167,
    3168,3169,3170,3171,3172,3174,3175,3176,3178,3179,
    3181,3182,3184,3185,3188,3189,3201,3203,3204,3205,
    3206,3207,3209,3210,3211,3212,3213,3214,3215,3216,
    3217,3218,3221,3222,3223,3224,3225,3226,3227,3228,
    3229,3230,3231,3232,3233,3234,3236,3237,3238,3239,
    3241,3242,3243,3245,3246,3247,3248,3249,3250,3251,
    3253,3254,3255,3258,3259,3260,3261,3262,3263,3264,
    3265,3266,3267,3268,3269,3271,3272,3273,3274,3275,
    3276,3278,3279,3280,3282,3284,3285,3286,3287,3288,
    3291,3294,3301,3305,3306,3309,3310,3311,3312,3313,
    3315,3316,3317,3318,3319,3320,3321,3322,3326,3328,
    3334,3335,3336,3337,3338,3339,3340,3342,3343,3346,
    3347,3348,3350,3351,3352,3353,3356,3359,3360,3361,
    3362,3364,3365,3366,3367,3369,3371,3372,3374,3375,
    3377,3380,3381,3382,3384,3385,3386,3387,3388,3390,
    3391,3392,3393,3394,3401,3402,3403,3404,3405,3406,
    3407,3408,3409,3411,3412,3413,3414,3415,3416,3417,
    3418,3419,3420,3424,3425,3426,3427,3428,3430,3431,
    3432,3436,3437,3438,3439,3444,3446,3448,3449,3451,
    3452,3453,3454,3456,3457,3458,3459,3462,3463,3464,
    3465,3466,3469,3470,3471,3472,3474,3476,3478,3480,
    3482,3483,3484,3485,3487,3489,3490,3502,3503,3505,
    3506,3510,3511,3516,3517,3518,3520,3521,3524,3526,
    3527,3528,3532,3534,3535,3536,3537,3538,3539,3540,
    3542,3543,3544,3545,3546,3547,3549,3551,3552,3553,
    3554,3555,3557,3559,3560,3561,3563,3564,3565,3566,
    3567,3568,3569,3570,3571,3572,3573,3574,3575,3576,
    3577,3578,3579,3582,3583,3588,3589,3590,3591,3602,
    3603,3604,3605,3606,3607,3608,3609,3611,3612,3614,
    3615,3616,3617,3618,3620,3622,3623,3625,3626,3627,
    3628,3630,3631,3632,3633,3634,3635,3636,3637,3638,
    3641,3643,3644,3645,3650,3658,3664,3666,3667,3668,
    3669,3671,3672,3673,3674,3675,3676,3677,3678,3679,
    3681,3683,3688,3689,3690,3691,3694,3701,3705,3706,
    3708,3709,3710,3711,3712,3715,3716,3717,3719,3720,
    3721,3723,3724,3725,3726,3730,3732,3733,3734,3736,
    3737,3739,3741,3744,3745,3747,3748,3751,3752,3756,
    3757,3758,3759,3760,3761,3762,3763,3764,3765,3766,
    3767,3771,3773,3774,3775,3776,3777,3779,3780,3782,
    3784,3786,3787,3789,3790,3791,3792,3793,3794,3804,
    3805,3806,3807,3808,3809,3811,3812,3813,3814,3815,
    3816,3817,3818,3819,3822,3824,3825,3827,3828,3829,
    3831,3832,3833,3835,3836,3838,3839,3840,3841,3844,
    3845,3847,3854,3858,3862,3863,3866,3876,3878,3880,
    3881,3883,3884,3885,3889,3892,3893,3894,3901,3902,
    3904,3905,3906,3907,3911,3913,3914,3915,3916,3919,
    3920,3926,3927,3928,3929,3930,3932,3936,3938,3940,
    3941,3943,3944,3947,3948,3949,3950,3951,3952,3953,
    3956,3957,3958,3959,3960,3961,3962,3963,3967,3968,
    3969,3970,3976,3978,3979,3981,3982,3985,3986,3987,
    3989,3990,3991,3994,4002,4009,4010,4012,4013,4015,
    4017,4018,4019,4020,4022,4028,4029,4030,4031,4033,
    4034,4036,4039,4040,4041,4042,4043,4044,4045,4046,
    4047,4049,4050,4051,4053,4054,4055,4058,4060,4061,
    4062,4064,4065,4066,4067,4068,4070,4071,4072,4073,
    4074,4075,4077,4078,4079,4081,4082,4083,4084,4087,
    4088,4089,4094,4101,4104,4112,4114,4117,4120,4122,
    4124,4125,4126,4128,4129,4130,4133,4134,4135,4136,
    4142,4145,4147,4148,4149,4150,4151,4152,4153,4155,
    4156,4157,4159,4160,4161,4162,4163,4165,4166,4167,
    4168,4169,4170,4172,4173,4174,4175,4177,4178,4180,
    4181,4184,4185,4187,4190,4191,4192,4193,4194,4201,
    4202,4203,4204,4207,4208,4209,4210,4212,4214,4215,
    4216,4219,4220,4222,4223,4224,4225,4226,4227,4228,
    4229,4230,4231,4232,4233,4234,4236,4237,4238,4241,
    4242,4244,4248,4249,4250,4252,4253,4254,4255,4256,
    4257,4259,4261,4262,4265,4266,4268,4270,4271,4272,
    4273,4275,4276,4277,4279,4280,4281,4283,4284,4285,
    4286,4288,4290,4292,4293,4301,4302,4304,4305,4306,
    4307,4311,4312,4313,4314,4316,4319,4320,4321,4322,
    4323,4324,4325,4326,4327,4328,4329,4330,4331,4332,
    4333,4334,4335,4336,4337,4339,4341,4344,4345,4347,
    4348,4349,4350,4355,4357,4359,4360,4361,4362,4363,
    4364,4365,4366,4367,4368,4369,4371,4372,4375,4376,
    4384,4385,4386,4390,4392,4393,4394,4401,4403,4404,
    4405,4408,4409,4414,4415,4417,4418,4419,4420,4421,
    4423,4424,4427,4428,4430,4431,4432,4433,4434,4435,
    4436,4439,4440,4442,4444,4447,4448,4449,4451,4452,
    4453,4454,4455,4458,4459,4460,4465,4468,4470,4471,
    4472,4475,4478,4480,4481,4482,4483,4484,4485,4486,
    4488,4491,4493,4494,4501,4502,4503,4505,4506,4507,
    4509,4510,4511,4513,4517,4519,4521,4522,4523,4525,
    4526,4527,4528,4529,4530,4531,4532,4534,4536,4538,
    4539,4540,4541,4542,4543,4545,4546,4547,4549,4550,
    4551,4553,4555,4556,4557,4559,4560,4562,4563,4565,
    4566,4567,4569,4571,4572,4574,4575,4577,4578,4579,
    4580,4581,4583,4584,4587,4588,4589,4590,4593,4601,
    4602,4604,4605,4606,4607,4608,4609,4612,4614,4617,
    4618,4619,4620,4621,4624,4625,4626,4627,4629,4630,
    4632,4633,4635,4636,4637,4640,4641,4644,4645,4646,
    4647,4648,4649,4650,4651,4655,4656,4657,4661,4662,
    4663,4664,4665,4667,4668,4669,4673,4675,4676,4677,
    4678,4679,4680,4681,4682,4683,4684,4685,4686,4687,
    4688,4693,4702,4703,4707,4709,4710,4711,4713,4715,
    4716,4718,4719,4723,4726,4727,4731,4732,4734,4735,
    4737,4739,4740,4749,4751 };

  if (JOYO_TOTAL != ARRAYSIZE(jk)) {
    fprintf(stderr,"Incomplete data structure for Joyo Kanji list! Exiting...\n");
    exit(1);
  }
  if (!suppress)
    fprintf(out,"MACHINE-READABLE JOYO KANJI LIST (in JIS order)\n\n");
  for (i = 0;i < JOYO_TOTAL;i++) {
    if (column == 0)
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          fprintf(out,"%s",ki);
          break;
        default :
          break;
      }
    if (wrap && column >= max) {
      switch (outcode) {
        case NEW :
        case OLD :
        case NEC :
          fprintf(out,"%s\n%s",ko,ki);
          break;
        case EUC :
        case SJIS :
          fprintf(out,"\n");
          break;
        default :
          break;
      }
      column = 0;
    }
    switch (outcode) {
      case NEW :
      case OLD :
      case NEC :
        fprintf(out,"%c%c",FIRSTBYTE(jk[i]), SECONDBYTE(jk[i]));
        column += 2;
        break;
      case EUC :
        fprintf(out,"%c%c",FIRSTBYTE(jk[i]) + 128, SECONDBYTE(jk[i]) + 128);
        column += 2;
        break;
      case SJIS :
        p1 = FIRSTBYTE(jk[i]);
        p2 = SECONDBYTE(jk[i]);
        jis2sjis(&p1,&p2);
        fprintf(out,"%c%c",p1,p2);
        column += 2;
        break;
      default :
        break;
    }
  }
  switch (outcode) {
    case NEW :
    case OLD :
    case NEC :
      fprintf(out,"%s\n",ko);
      break;
    case EUC :
    case SJIS :
      fprintf(out,"\n");
      break;
    default :
      break;
  }
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
