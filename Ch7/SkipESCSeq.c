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
