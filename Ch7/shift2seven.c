void shift2seven(FILE *in,FILE *out,int incode,char ki[],char ko[])
{
  int p1;
  int p2;
  int intwobyte = FALSE;

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
