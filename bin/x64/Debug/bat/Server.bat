:: 命令脚本支持2.5及以上版本
@set strIP=127.0.0.1
@set cmd=strIP=%strIP%
@set cmd=%cmd% nPort=4568
@set cmd=%cmd% nThread=4
@set cmd=%cmd% nClient=1000
EasyTcpServer2.5 %cmd%
pause