:: 命令脚本支持2.5及以上版本
@set strIP=127.0.0.1
@set cmd=strIP=%strIP%
@set cmd=%cmd% nPort=4568
@set cmd=%cmd% nThread=4
@set cmd=%cmd% nMaxClient=10240

@set cmd=%cmd% nSendBuffSize=81920
@set cmd=%cmd% nRecvBuffSize=81920
@set cmd=%cmd% -sendback
@set cmd=%cmd% -sendfull
@set cmd=%cmd% -checkMsgID
@set cmd=%cmd% -p

EasyTcpServer2.6 %cmd%

pause