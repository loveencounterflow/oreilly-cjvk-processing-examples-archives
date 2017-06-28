void jis2sjis(int *p1, int *p2)
{
  unsigned char c1 = *p1;
  unsigned char c2 = *p2;
  int rowOffset = c1 < 95 ? 112 : 176;
  int cellOffset = c1 % 2 ? (c2 > 95 ? 32 : 31) : 126;

  *p1 = ((c1 + 1) >> 1) + rowOffset;
  *p2 += cellOffset;
}
