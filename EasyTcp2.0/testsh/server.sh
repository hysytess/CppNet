strIP='127.0.0.1'
nPort=4568
nThread=4
nClient=1000

cmd='strIP='$strIP''' nPort='$nPort''' nThread='$nThread''' nClient='$nClient''

./server2.5 $cmd

read -p "..press and key to exit .." var
