::@echo off
::@set ip=127.0.0.1 
::@set port=4567
::@set threadcount=1
::@set clientcount=100

: "@cmdtest 不显示绝对路径"
: "cmdtest 显示绝对路径"
::cmdtest %ip% %port% %threadcount% %clientcount%

@echo off
set ip=127.0.0.1 
set port=4567
set threadcount=1
set clientcount=100

::set cmd=ip=%ip% port=%port% threadcount=%threadcount% clientcount=%clientcount%

set cmd=-ip %ip% -port %port% -threadcount %threadcount% -clientcount %clientcount%
Cmdtest_main %cmd%
pause