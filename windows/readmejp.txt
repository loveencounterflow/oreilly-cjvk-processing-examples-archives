This is japanese(S-JIS) text.
English texts available on WinJConv.c.

		WinJConv.dll(ver 0.80)�ɂ���

�P�D�͂��߂�

  �{�v���O�����́A�u���{���񏈗��v�̒��҂ł���Ken R. Lunde��
  �쐬�̓��{�ꕶ���Z�b�g�ϊ��v���O����JCONV���AWindows95/NT��
  DLL�Ƃ��ĈڐA�������̂ł��B

  �{�v���O�����̓t���[�E�G�A�Ƃ��Ă̔z�z�A�y�у\�[�X�R�[�h�̏C���^
  �ύX���͎��R�ł����A�g�p�̍ۂ́A�g�p�҂̐ӔC�ɂ����čs���ĉ������B
  ���ALunde���ɂ��z�z�����^����͖{�v���O�����ł����������
  �l���Ă��܂��B �i�m�F�͂Ƃ��Ă��܂��񂪁j

  �����Ɍ����̂܂܋L�ڂ��Ă����܂��B

  DISTRIBUTION AND RESTRICTIONS ON USAGE:
   1) Please give this source code away to your friends at no charge.
   3) Only use this tool on a copy of a file -- do not use an original. This
      is just common sense.
   4) This source code or a compiled version may be bundled with commercial
      software as long as the author is notified beforehand. The author's name
      should also be mentioned in the credits.
   5) Feel free to use any of the algorithms for your own work. Many of them are
      being used in other tools I have written.
  �����A�I���W�i����Windows�p�ɕύX�������_�ŁA����̂���������
  �͗L���ł͂Ȃ���������܂���B
  �C�ɂȂ����Lunde���ɘA��������ĉ������B

  ���A�I���W�i���Ɠ��l�ɖ{�v���O���������p�Ŏg�p����ꍇ�́A
  �i���j�L���X�g ���Ăɂ��A���������B

�Q�DWinJConv�͉��������Ȃ����H

   ���{��̃e�L�X�g�t�@�C����ǂ݁A�w�肳�ꂽ�����R�[�h�ɕϊ����A
   �^����ꂽ�o�b�t�@�ɃZ�b�g���܂��B

   �ϊ��\�ȕ����Z�b�g�i���o�͋��j
 	Shift-JIS, EUC, New-JIS, Old-JIS, NEC-JIS 
   ���̓t�@�C���̕����R�[�h�����F���i�����j�@�\�t��
	������EUC�R�[�h��ǂ܂������ɁAEUC��S-JIS�̌��肪�o���Ȃ��ꍇ
	�������̂ŁA���ӂ��K�v�ł��B���i�i���j�́AWinJConv��EUC/S-JIS�̔F��
	���o���Ȃ��ꍇ�́A���͂�EUC�Ƃ��čēx�ϊ��������Ȃ킹�Ă��܂��B
   ���̑��̋@�\
	���p�ł̑S�p�J�i�ւ̕ύX(���o�͂̕����R�[�h������S-JIS/EUC�̎��j
	�j�������Ǝv����OLD/NEW-JIS�e�L�X�g�̏C��
	OLD/NEW-JIS�̃G�X�P�[�v�����̍폜


�R�D�����R�[�h�ϊ��̍s������
   ���C���v���O�������玟�̓��e���Z�b�g���ăR�[�����ĉ������B
  
   int WinJConv(LPSTR infilename, LPSTR out, int funccode, int incode, int outcode)
	
	�߂�l:	-1=�@�\�R�[�h���́A���͕����R�[�h������Ă���B
		-2=�t�@�C����������Ȃ��B���̓t�@�C���̕����R�[�h������o���Ȃ��B
		-3=���͕����R�[�h�������ŔF�������鎞�A
		   Shift-JIS �� EUC�̊e�ꂩ�ɔ���o�����������B
		��L�ȊO=���̓t�@�C���̕����R�[�h�i���L�Q�Ɓj
	infilename:���̓e�L�X�g�t�@�C����
	out:�o�̓o�b�t�@�[
		�K�v�Ȓ������m�ۂ��ANULL�ŏI�[���ĉ������B
		�i�A���o�b�t�@�[�̒����S��NULL����WinJConv�͉����s���܂���B�j
		�ϊ��̓t�@�C����ǂݍ��ݏI��邩�AOUT�o�b�t�@�[�̒���NULL��
		�����������_�ŏI�����܂��B	
	funccode:�@�\�R�[�h	
		0=�P���ϊ�
		1=�����R�[�h�ϊ��y�є��p��->�S�p�J�i�ϊ�
		2=OLD/NEW-JIS�R�[�h�t�@�C���̏C��
		3=�G�X�P�[�v�����̍폜(OLD/NEW-JIS�̂�)
		4=�G�X�P�[�v�����̋����폜(OLD/NEW-JIS�̂�)
		5=���̓t�@�C���̕����R�[�h�̎�������
			�i�ϊ����Ȃ��A�o�̓o�b�t�@�[�ɂ͂Ȃɂ��Z�b�g����Ȃ��j
		9=�{DLL�̃f�o�b�O�p
			�>�����̃g���[�X���ʂ��o�̓o�b�t�@�ɏ����o���܂��B
	incode/outcode:�����R�[�h
		0=incode�ɗ^�������A���̓t�@�C���̕����R�[�h�������ŔF��
		   outcode�ɗ^�������A�o�͂��f�t�H���g��S-JIS�ɕϊ�
		1=NEW-JIS
		2=OLD-JIS
		3=NEC
		4=EUC
		5=SJIS
		6=EUCORSJIS
		7=ASCII

�S�D�o�[�W�����ɂ���
   �����_�ɂ����ẮA���L�̋@�\�̃e�X�g���������Ă��܂��B
	�P���ϊ� �y�є��p��->�S�p�J�i�ϊ�
		NEW-JIS	->  S-JIS
		EUC	->  S-JIS
		S-JIS	->  S-JIS
	OLD/NEW-JIS�R�[�h�t�@�C���̏C��
		�m�F���Ă��܂���B
	�G�X�P�[�v�����̍폜�^�����폜
		�m�F���Ă��܂���B
	���̓t�@�C���̕����R�[�h�̎�������
		NEW-JIS, EUC,S-JIS
   ���̋@�\�ɂ��ĂǂȂ����m�F�����ꂽ��A���i�i���j���ĂɘA�����������B
   �m�F���S�������������_�Ńo�[�W������1.0�ɂ��悤�ƍl���Ă��܂��B

�T�D�R���p�C����
    MicroSoft Visual C++ ���{��� 4.0�ŃR���p�C�����s���܂����B
    WinJConv.c��WinJConv.def����̃v���W�F�N�g�Ɋ܂߂ăr���h���ĉ������B
    �I���W�i����ANSI-C�����̃|�[�^�u���Ȍ`�ō쐬����Ă܂��̂ŁA
    �W���I��DLL���g�p���郉�C�u�����ȊO�͉���(MFC��)�K�v����܂���B

�U�D���̑��̏��
    ���{���񏈗��Ɋւ��Ă�,
	Ken R. Lude���̃z�[���E�y�[�W->	http://jasper.ora.com/lunde/
				 ����->	���{���񏈗�
					�\�t�g�o���N�i���j4,800�~
�V�D�o�O���̑��̖₢���킹
	E-MAIL:	nagase@cast.co.jp
	MAIL:	�����s�䓌�搼�󑐂R�[�Q�U�[�U
		�i���j�L���X�g
			�i�� ���m

�W�D�ŐV�ł̓�����@
    ���̂t�q�k�������o���܂��B
	ftp://ftp.cast.co.jp/pub/tools/japanese	
						->�i���j�L���X�g�̂e�s�o�T�C�g�ł��B
	http://www.cast.co.jp
						->�i���j�L���X�g��WEB�T�C�g�ł��B
						  �i�����܂���B���ݍH�����ł��B�j
	ftp://ftp.ora.com/pub/examples/nutshell/ujip/windows/
						->�č�O'REILLY�Ђ̂e�s�o�T�C�g�B
		  			  	  Lunde���ɂ���ēo�^����܂��B


					1996�N�V���P�X�i���j�L���X�g �i��
