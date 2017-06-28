void euc2shift(FILE *in,FILE *out,int incode,int tofullsize)
{
  int p1;
  int p2;

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
