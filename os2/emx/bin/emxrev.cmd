/* emxrev.cmd */

  call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
  call SysLoadFuncs

  parse arg arg1 rest
  select
  when arg1 = '' then do
    say revision( 'emx')
    say revision( 'emxio')
    say revision( 'emxlibc')
    say revision( 'emxwrap')
  end
  when arg1 = '-a' then do
    if rest \= '' then
      call usage
    call find_all
  end
  when arg1 = '-c' then do
    if (rest = '') | (words( rest) > 1) then
      call usage
    parse var rest drive':'rest
    if (length( drive) \= 1) | (rest \= '') then
      call usage
    call find_tree( drive':')
  end
  when arg1 = '-d' then do
    if (rest = '') | (words( rest) > 1) then
      call usage
    call find_dir( rest)
  end
  when arg1 = '-f' then do
    if (rest = '') | (words( rest) > 1) then
      call usage
    say revision( rest)
  end
  when arg1 = '-p' then do
    if (rest = '') | (words( rest) > 1) then
      call usage
    call find_config rest
  end
  otherwise
    call usage
  end
  exit 0

usage:
  say 'Usage:'
  say '  emxrev           Display revision number of default emx DLLs'
  say '  emxrev -a        Scan all disks for emx DLLs'
  say '  emxrev -c d:     Scan complete drive D: for emx DLLs'
  say '  emxrev -d dir    Scan directory DIR for emx DLLs'
  say '  emxrev -f file   Display revision number of FILE'
  say '  emxrev -p file   Scan directories in LIBPATH statement of FILE'
  say ''
  say 'emx DLLs are emx.dll, emxio.dll, emxlibc.dll and emxwrap.dll'
exit 1

find_all: procedure
  drives = SysDriveMap()
  do i = 1 to words( drives)
    call find_tree( word( drives, i))
  end
  return

find_tree: procedure
  arg drive
  call find_dir drive'\'
  return

find_dir: procedure
  arg dir
  last = right( dir, 1)
  if (last \= '/') & (last \= '\') & (last \= ':') then
    dir = dir'\'
  call SysFileTree dir'emx*.dll', files, 'FSO'
  do i = 1 to files.0
    name = filespec( 'name', files.i)
    if (name = 'EMX.DLL') | (name = 'EMXIO.DLL') | (name = 'EMXLIBC.DLL'),
       | (name = 'EMXWRAP.DLL') then
      say revision( files.i)
  end
  return
  
find_config: procedure
  arg config
  call SysFileSearch 'LIBPATH=', config, lines
  do i = 1 to lines.0
    parse var lines.i 'LIBPATH='rest
    list = translate( rest, ' ', ';')
    do j = 1 to words( list)
      call find_dir word( list, j)
    end
  end
  return

revision: procedure
  arg pathname
  call RxFuncAdd emx_revision, pathname, emx_revision
  signal on syntax name error
  tmp = emx_revision()
  signal off syntax
  call RxFuncDrop emx_revision
  return pathname ': revision =' tmp

error:
  return pathname ': revision number not available'
