echo off
CLS
echo **********************************************************************
echo * Thank you for your interest in KanjiWORD 3.0,                      *
echo * the Japanese word processor for English Windows!                   *
echo *                                                                    *
echo * Please wait while the files are copied and extracted.              *
echo *                                                                    *
echo * After installation, you will automatically return to               *
echo * Windows and the KanjiWORD Demo will begin.                         *
echo *                                                                    *
echo * To view the demo again, just press the Play button                 *
echo * on the Lotus Screencam in the lower right-hand corner.             *
echo *                                                                    *
echo * You can also run the demo by double-clicking on the                *
echo * DEMO.EXE file in the C:\KWW-DEMO directory in the File Manager.    *
echo **********************************************************************
echo           Installing KanjiWORD 3.0 Self Running Demo
echo              to your hard drive in C:\KWW-DEMO.
md C:\KWW-DEMO
copy KW3DEMO.EXE C:\KWW-DEMO
c:\
cd\
cd kww-demo
KW3DEMO
echo Installation Complete.
cd\
win c:\kww-demo\demo /s