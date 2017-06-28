This is japanese(S-JIS) text.
English texts available on WinJConv.c.

		WinJConv.dll(ver 0.80)について

１．はじめに

  本プログラムは、「日本語情報処理」の著者であるKen R. Lunde氏
  作成の日本語文字セット変換プログラムJCONVを、Windows95/NTの
  DLLとして移植したものです。

  本プログラムはフリーウエアとしての配布、及びソースコードの修正／
  変更等は自由ですが、使用の際は、使用者の責任において行って下さい。
  又、Lunde氏による配布条件／制約は本プログラムでも成立すると
  考えています。 （確認はとっていませんが）

  ここに原文のまま記載しておきます。

  DISTRIBUTION AND RESTRICTIONS ON USAGE:
   1) Please give this source code away to your friends at no charge.
   3) Only use this tool on a copy of a file -- do not use an original. This
      is just common sense.
   4) This source code or a compiled version may be bundled with commercial
      software as long as the author is notified beforehand. The author's name
      should also be mentioned in the credits.
   5) Feel free to use any of the algorithms for your own work. Many of them are
      being used in other tools I have written.
  ただ、オリジナルをWindows用に変更した時点で、制約のうちいくつか
  は有効ではないかもしれません。
  気になる方はLunde氏に連絡を取って下さい。

  又、オリジナルと同様に本プログラムを商用で使用する場合は、
  （株）キャスト 宛てにご連絡下さい。

２．WinJConvは何をおこなうか？

   日本語のテキストファイルを読み、指定された文字コードに変換し、
   与えられたバッファにセットします。

   変換可能な文字セット（入出力共）
 	Shift-JIS, EUC, New-JIS, Old-JIS, NEC-JIS 
   入力ファイルの文字コード自動認識（検査）機能付き
	ただしEUCコードを読ませた時に、EUCとS-JISの決定が出来ない場合
	が多いので、注意が必要です。私（永瀬）は、WinJConvがEUC/S-JISの認識
	が出来ない場合は、入力をEUCとして再度変換をおこなわせています。
   その他の機能
	半角ｶﾅの全角カナへの変更(入出力の文字コードが共にS-JIS/EUCの時）
	破損したと思われるOLD/NEW-JISテキストの修復
	OLD/NEW-JISのエスケープ文字の削除


３．文字コード変換の行いかた
   メインプログラムから次の内容をセットしてコールして下さい。
  
   int WinJConv(LPSTR infilename, LPSTR out, int funccode, int incode, int outcode)
	
	戻り値:	-1=機能コード又は、入力文字コードが誤っている。
		-2=ファイルが見つからない。入力ファイルの文字コードが決定出来ない。
		-3=入力文字コードを自動で認識させる時、
		   Shift-JIS と EUCの各れかに判定出来無かった。
		上記以外=入力ファイルの文字コード（下記参照）
	infilename:入力テキストファイル名
	out:出力バッファー
		必要な長さを確保し、NULLで終端して下さい。
		（但しバッファーの中が全てNULLだとWinJConvは何も行いません。）
		変換はファイルを読み込み終わるか、OUTバッファーの中にNULLを
		発見した時点で終了します。	
	funccode:機能コード	
		0=単純変換
		1=文字コード変換及び半角ｶﾅ->全角カナ変換
		2=OLD/NEW-JISコードファイルの修復
		3=エスケープ文字の削除(OLD/NEW-JISのみ)
		4=エスケープ文字の強制削除(OLD/NEW-JISのみ)
		5=入力ファイルの文字コードの自動検査
			（変換しない、出力バッファーにはなにもセットされない）
		9=本DLLのデバッグ用
			ｰ>処理のトレース結果を出力バッファに書き出します。
	incode/outcode:文字コード
		0=incodeに与えた時、入力ファイルの文字コードを自動で認識
		   outcodeに与えた時、出力をデフォルトのS-JISに変換
		1=NEW-JIS
		2=OLD-JIS
		3=NEC
		4=EUC
		5=SJIS
		6=EUCORSJIS
		7=ASCII

４．バージョンについて
   現時点においては、下記の機能のテストが完了しています。
	単純変換 及び半角ｶﾅ->全角カナ変換
		NEW-JIS	->  S-JIS
		EUC	->  S-JIS
		S-JIS	->  S-JIS
	OLD/NEW-JISコードファイルの修復
		確認していません。
	エスケープ文字の削除／強制削除
		確認していません。
	入力ファイルの文字コードの自動検査
		NEW-JIS, EUC,S-JIS
   他の機能についてどなたか確認をされたら、私（永瀬）宛てに連絡を下さい。
   確認が全部完了した時点でバージョンを1.0にしようと考えています。

５．コンパイル環境
    MicroSoft Visual C++ 日本語版 4.0でコンパイルを行いました。
    WinJConv.cとWinJConv.defを一つのプロジェクトに含めてビルドして下さい。
    オリジナルがANSI-C準拠のポータブルな形で作成されてますので、
    標準的なDLLが使用するライブラリ以外は何も(MFCも)必要ありません。

６．その他の情報
    日本語情報処理に関しては,
	Ken R. Lude氏のホーム・ページ->	http://jasper.ora.com/lunde/
				 著書->	日本語情報処理
					ソフトバンク（株）4,800円
７．バグその他の問い合わせ
	E-MAIL:	nagase@cast.co.jp
	MAIL:	東京都台東区西浅草３ー２６ー６
		（株）キャスト
			永瀬 徳仁

８．最新版の入手方法
    次のＵＲＬから入手出来ます。
	ftp://ftp.cast.co.jp/pub/tools/japanese	
						->（株）キャストのＦＴＰサイトです。
	http://www.cast.co.jp
						->（株）キャストのWEBサイトです。
						  （すいません。現在工事中です。）
	ftp://ftp.ora.com/pub/examples/nutshell/ujip/windows/
						->米国O'REILLY社のＦＴＰサイト。
		  			  	  Lunde氏によって登録されます。


					1996年７月１９（株）キャスト 永瀬
