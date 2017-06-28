void repairjis(FILE *in,FILE *out)
{
  int p1;
  int p2;
  int p3;
  int shifted_in = FALSE;

  while ((p1 = getc(in)) != EOF) {
    if (shifted_in) {
      if (p1 == ESC) {
        p2 = getc(in);
        if (p2 == '(') {
          p3 = getc(in);
          switch (p3) {
            case 'J' :
            case 'B' :
            case 'H' :
              shifted_in = FALSE;
              break;
            default :
              break;
          }
          fprintf(out,"%c%c%c",p1,p2,p3);
        }
      }
      else if (p1 == '(') {
        p2 = getc(in);
        switch (p2) {
          case 'J' :
          case 'B' :
          case 'H' :
            shifted_in = FALSE;
            fprintf(out,"%c%c%c",ESC,p1,p2);
            break;
          default :
            fprintf(out,"%c%c",p1,p2);
            break;
        }
      }
      else {
        p2 = getc(in);
        fprintf(out,"%c%c",p1,p2);
      }
    }
    else {
      if (p1 == ESC) {
        p2 = getc(in);
        if (p2 == '$') {
          p3 = getc(in);
          switch (p3) {
            case 'B' :
            case '@' :
              shifted_in = TRUE;
              break;
            default :
              break;
          }
          fprintf(out,"%c%c%c",p1,p2,p3);
        }
      }
      else if (p1 == '$') {
        p2 = getc(in);
        switch (p2) {
          case 'B' :
          case '@' :
            shifted_in = TRUE;
            fprintf(out,"%c%c%c",ESC,p1,p2);
            break;
          default :
            fprintf(out,"%c%c",p1,p2);
            break;
        }
      }
      else
        fprintf(out,"%c",p1);
    }
  }
}
