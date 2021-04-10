# CppNet

## 介绍
> 这是一个简易跨平台TCP/UDP服务器程序，仓库会同步每个版本的迭代进度。

- EasyTcpClientGO、1.0-1.2 版本：简易 一对一 winsocket 服务器程序.

- 1.3-1.4 版本：加入跨平台特性,引入 select I/O 多路复模型,服务端加入接收缓冲区,客户端加入接收缓冲区和消息缓冲区,提升服务器客户端整体性能,Windows下服务器可以处理最多64(Linux最大连接数1024)个客户端连接和数据收发.单个客户端可以在Windows下最高可以有64(Linux最大连接数1024)个Socket批量链接服务器并进行简单数据的收发.
- 1.5:突破 Windows 下 select 最大连接数限制,封装服务端/客户端 客户端支持并发连接.
- 1.6 版本: 调整了服务器架构,引入生产者-消费者模型,分离接受连接,数据收发模块.服务端加入多线程和连接缓冲区,提高连接受理速度,引入高精度计时器,服务端可以实时显示当前已连入客户端数量和每秒处理数据包的的数量.
- 1.7 


## 使用教程

1.  下载
2.  解压
3.  启动EasyTcpServer
4.  启动EasyTcpClient

## 使用说明

1.  默认端口:4567
2.  服务端默认Ip:127.0.0.1
3.  命令行启动:
- windows: server.exe/client.exe ip port n [n: 1.服务端可承载连接数量 2.客户端连接数量]
- Linux/Unix: ./server./client ip port n [n: 1.服务端可承载连接数量 2.客户端连接数量]

## 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


## 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
