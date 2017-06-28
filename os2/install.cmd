/* Script to install JList */

/* Install the needed functions from RexxUtil */
call RxFuncAdd 'SysCreateObject','RexxUtil','SysCreateObject'
call RxFuncAdd 'SysIni','RexxUtil','SysIni'
call RxFuncAdd 'SysFileTree','RexxUtil','SysFileTree'
call RxFuncAdd 'SysDriveMap','RexxUtil','SysDriveMap'

/* Search the executable */
exe=find('jlist.exe')
if exe='' then
do
  say 'JLIST.EXE could not be found. Aborting.'
  exit
end

say 'Installing 'exe

/* Create a program object for JList on the desktop */
if SysCreateObject('WPProgram','JList','<WP_DESKTOP>','EXENAME='exe';OBJECTID=<JLIST>','u') then
  say 'A JList program object has been created on the desktop.'
else
  say 'The JList program object could not be created.'

/* If the system doesn't know the file type Kanjifont, we tell him
   about it. Setting the USER_INI entry is a little cheating, but i
   don't know a 'legal' way to create a new type without an assosiated
   program */
if SysIni(,'PMWP_ASSOC_TYPE','Kanji Font')='ERROR:' then
  call SysIni ,'PMWP_ASSOC_TYPE','Kanji Font','00'x

/* Now we set the fonts (if they are not already set) */
/* Courier is a nice ASCII font to use, as it is monospaced */
if SysIni(,'JList','AsciiFontName')='ERROR:' then
do
  call SysIni ,'JList','AsciiFontName','Courier'"00"x
  call SysIni ,'JList','AsciiFontStyle','0000'x
end

/* Search the kanji font */
dfont=find('k16x16.f00')
if dfont='' then
do
  say 'The kanji font could not be found. You will be able to run JList,'
  say 'but before you can display anything you have to open a kanji font.'
end
else 
do
  say 'Using the kanji font 'dfont
  if SysIni(,'JList','KanjiDispFont')='ERROR:' then
    call SysIni ,'JList','KanjiDispFont',dfont"00"x
end

exit

find:
arg file

/* First we take a look into the current directory */
call SysFileTree file,files,'fo'
if files.0\=0 then 
  return files.1

/* If we didn't find it there, the current drive is next */
call SysFileTree left(directory(),2)'\'file,files,'fos'
if files.0\=0 then 
  return files.1

/* Still nothing? Then we search all other drives */
drives=SysDriveMap()
do drive=1 to words(drives)
  /* We have already searched the current drive */
  if word(drives,drive)\=left(directory(),2) then
    call SysFileTree word(drives,drive)'\'file,files,'fos'
  if files.0\=0 then
    return files.1
end

/* Ok, i give up */
return ''
