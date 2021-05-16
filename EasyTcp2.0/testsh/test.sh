#服务端IP
ip=127.1.1.12
#服务端端口
port=4567
#消息处理线程数
threadcount=4
#客户端连接上限
clientcount=100
cmd='-ip'
cmd="$cmd $ip"
cmd="$cmd -port $port"
cmd="$cmd -threadcount $threadcount"
cmd="$cmd -clientcount $clientcount"

./shtest $cmd