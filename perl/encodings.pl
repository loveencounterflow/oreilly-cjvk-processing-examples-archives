$encoding = qq<[\x00-\xFF][\x00-\xFF]>; # UCS-2
$encoding = qq< # EUC-JP
  [\x00-\x8D\x90-\xA0\xFF]|   # Code set 0 & one-byte
  \x8E[\xA0-\xDF]|            # Code set 2
  \x8F[\xA1-\xFE][\xA1-\xFE]| # Code set 3
  [\xA1-\xFE][\xA1-\xFE]      # Code set 1
>;
$encoding = qq< # EUC-TW
  [\x00-\x8D\x8F-\xA0\xFF]|              # Code set 0 & one-byte
  \x8E[\xA1-\xB0][\xA1-\xFE][\xA1-\xFE]| # Code set 2
  [\xA1-\xFE][\xA1-\xFE]                 # Code set 1
>;
$encoding = qq< # EUC-KR and EUC-CN
  [\x00-\xA0\xFF]|       # Code set 0 & one-byte
  [\xA1-\xFE][\xA1-\xFE] # Code set 1
>;
$encoding = qq< # GBK
  [\x00-\x80\xFF]|               # One-byte
  [\x81-\xFE][\40-\x7E\x80-\xFE] # GBK
>;
