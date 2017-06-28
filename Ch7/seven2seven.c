void seven2seven(FILE *in,FILE *out,char ki[],char ko[])
{
  int temp;
  int p1;
  int p2;
  int change;
  int intwobyte = FALSE;

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
