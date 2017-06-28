#
# 漢字コード変換ライブラリ codeconv.pl
#

package codeconv;
sub main'jis2sjis { &conv($_[0], "jis2sjis"); }
sub main'sjis2jis { &conv($_[0], "sjis2jis"); }
sub main'jis2ujis { &conv($_[0], "jis2ujis"); }
sub main'ujis2jis { &conv($_[0], "ujis2jis"); }

$main'ASCIIPAT = '[^\033\200-\377]+';
$main'JISPAT = '\033\$.(([^\033].)*)\033\(.'; # JIS文字列にマッチするパタン文字列
$main'EUCPAT = '([\200-\377].)+';             # EUC/SJIS文字列にマッチ
sub conv {
	local($_, $func) = @_;
	local($s);
	local($jtox) = ($func =~ /^j/);  # jis2something
	while($_ ne ''){
		$s .= s/^$main'ASCIIPAT// ? $& :
		      s/^$main'EUCPAT// ? ($jtox ? $& : do $func($&)) :
		      s/^$main'JISPAT// ? ($jtox ? do $func($1) : $&) :
		      s/^.// , $& ;
	}
	$s;
}
sub sjis2jis {
	local($_) = @_;
	local($s, $c1, $c2, $j1, $j2);
	while(s/^..//){
		($c1,$c2) = unpack("CC",$&);
		$c1 -= 0x40 if $c1 >= 0xe0;
		$c2-- if $c2 >= 0x80;
		$j1 = ($c1-0x81) * 2 + ($c2>=0x9e ? 1 : 0) + 0x21;
		$j2 = ($c2 >= 0x9e ? $c2-0x9e : $c2-0x40) + 0x21;
		$s .= pack("CC",$j1,$j2);
	}
	"\033\$B" . $s . "\033(B" ;
}
sub jis2sjis {
	local($_) = @_;
	local($s, $c1, $c2, $j1, $j2);
	while(s/^..//){
		($c1,$c2) = unpack("CC",$&);
		$c2 = ($c1 & 1 ? $c2-0x21+0x40 : $c2-0x21+0x9e);
		$c1 = ($c1 - 0x21) / 2 + 0x81;
		$j1 = ($c1 >= 0xa0 ? $c1 + 0x40 : $c1);
		$j2 = ($c2 >= 0x7f ? $c2 + 1 : $c2);
		$s .= pack("CC",$j1,$j2);
	}
	$s ;
}
sub ujis2jis {
	local($_) = @_;
	local($s);
	while(s/^.//){
		$s .= pack("C", unpack("C",$&) & 0x7f);
	}
	"\033\$B" . $s . "\033(B" ;
}
sub jis2ujis {
	local($_) = @_;
	local($s);
	while(s/^.//){
		$s .= pack("C", unpack("C",$&) | 0x80);
	}
	$s ;
}
1;
