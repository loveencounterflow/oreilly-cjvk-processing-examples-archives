/* These are for the text stream handling algorithms. */
#define NL           10
#define CR           13
#define ESC          27
#define SS2          142
#define TRUE         1
#define FALSE        0
#define SJIS1(A)     ((A >= 129 && A <= 159) || (A >= 224 && A <= 239))
#define SJIS2(A)     (A >= 64 && A <= 252)
#define HANKATA(A)   (A >= 161 && A <= 223)
#define ISEUC(A)     (A >= 161 && A <= 254)

FILE *in;
FILE *out;

/* These are for the miscellaneous algorithms. */
#define NEW          1
#define OLD          2
#define NEC          3
#define EUC          4
#define SJIS         5
#define EUCORSJIS    6
#define ASCII        7
#define ISMARU(A)    (A >= 202 && A <= 206)
#define ISNIGORI(A)  ((A >= 182 && A <= 196) || (A >= 202 && A <= 206) || (A == 179))
