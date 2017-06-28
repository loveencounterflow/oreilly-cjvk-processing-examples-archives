/* Script to delete the WPS object and the USER_INI entries created by
   install.cmd for JList */

/* Install the needed functions from RexxUtil */
call RxFuncAdd 'SysDestroyObject','RexxUtil','SysDestroyObject'
call RxFuncAdd 'SysIni','RexxUtil','SysIni'

/* Remove the WPS object */
if SysDestroyObject('<JLIST>') then
  say 'The JList program object has been removed.'
else
  say 'Could not remove the JList program object.'

/* Delete the USER_INI entries */
call SysIni 'USER','JList'
