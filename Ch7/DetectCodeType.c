int DetectCodeType(FILE *in)
{
  int c = 0;
  int whatcode = ASCII;

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
