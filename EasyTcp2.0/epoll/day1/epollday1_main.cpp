#include <stdio.h>
#include <cstdlib>
#include <unistd.h> //uni std
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>

#include <vector>
#include <thread>
#include <algorithm>

#include "MessageHeader.hpp"

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#define MAX_EVENTS 256

std::vector<SOCKET> g_clients;
char g_szBuff[4096]{};
int g_Len = 0;
bool g_bRun = true;

void cmdThread()
{
	while (true)
	{
		char cmdBuf[256];
		scanf("%s", cmdBuf);
		if (!strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			break;
		}
	}
}

int cell_epoll_ctl(int epfd, int opt, SOCKET csock, uint32_t events)
{
    epoll_event ev;
    ev.events = events;
    ev.data.fd = csock;
    if(-1 == epoll_ctl(epfd,opt,csock,&ev))
    {
        printf("error, epoll_ctl() socket<%d>\n",csock);
    }
}

int cell_epoll_del(int epfd, int opt, SOCKET csock, epoll_event* events)
{
    epoll_ctl(epfd,opt,csock,events);   
}


int readData(SOCKET csock)
{
    int nLen = (int)recv(csock,g_szBuff,4096,0);
    g_Len = nLen;
    if (nLen <= 0)
    {
        return SOCKET_ERROR;
    }
    return nLen;
    
}

int writeData(SOCKET csock)
{
    int nLen = (int)send(csock,g_szBuff,g_Len,0);
    if (nLen <= 0)
    {
        return SOCKET_ERROR;
    }
    g_Len = 0;
    return nLen;   
}

int clientLeave(SOCKET csock)
{
    printf("client<%d> was exit.\n",csock);
    close(csock);
    auto iter = std::find(g_clients.begin(),g_clients.end(),csock);
    
    if (g_clients.begin()!=iter)
    {
        g_clients.erase(iter);
    }
    
}

int main(int argc,char* args[])
{
    std::thread t(cmdThread);
	t.detach();

    const char* ip = nullptr;
    unsigned short port = 4568;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    int on = 1;
    setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

    sockaddr_in _sin{};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);

    if (ip)
		_sin.sin_addr.s_addr = inet_addr(ip);
	else
		_sin.sin_addr.s_addr = INADDR_ANY;
	
    int ret = bind(sock, (sockaddr*)&_sin, sizeof(_sin));
	if (SOCKET_ERROR == ret)
		printf("Error, bind port<%d> fail...\n", port);
	else
		printf("Port<%d> bind sucess...\n", port);

    int listenret = listen(sock, 64);
	if (listenret == SOCKET_ERROR)
		printf("<socket=%d> listen error...\n", (int)sock);
	else
		printf("<socket=%d> listen, wait for client connect...\n", (int)sock);

    // 2.6.8 后 __size 参数无意义 最大连接数取决于硬件条件
    // 最大连接数由 epoll 动态管理.
    // cat /proc/sys/fs/file-max
    int epfd = epoll_create(MAX_EVENTS);

    cell_epoll_ctl(epfd,EPOLL_CTL_ADD,sock,EPOLLIN);

    int msgCount = 0;
    epoll_event events[MAX_EVENTS] = {};
    while (g_bRun)
    {
        int n = epoll_wait(epfd, events, MAX_EVENTS, 1/*-1 0 unlimit ms*/);

        if (n < 0)
        {
	        printf("error epoll_wait ret=%d \n",n);
            break;
        }
      
        for(int i = 0;i < n;i++)
        {
            if (events[i].data.fd == sock)
            {
                // read
                if(events[i].events & EPOLLIN)
                {
                    sockaddr_in clientAddr{};
            	    int nAddrLen = sizeof(sockaddr_in);
	                SOCKET csock = INVALID_SOCKET;

                    csock = accept(sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);

	                if (INVALID_SOCKET == csock)
		                printf("socket=<%d> error, invalid SOCKET...\n", (int)csock);
	                else
                    {
                        g_clients.push_back(csock);
                        // 将客户端添加到epoll集合中 让epoll管理
                        cell_epoll_ctl(epfd,EPOLL_CTL_ADD,csock,EPOLLIN);
	                    printf("New client join, socket = %d, IP = %s \n", (int)csock, inet_ntoa(clientAddr.sin_addr));
                    }
                    continue;
                }      
            }

            // read 
            if(events[i].events & EPOLLIN)
            {
                printf("EPOLLIN|%d\n",++msgCount);
                SOCKET csock = events[i].data.fd;
                int ret = readData(csock);
                if (ret <= 0)
                {
                    clientLeave(csock);
                cell_epoll_ctl(epfd,EPOLL_CTL_DEL,csock,0);
                }
                else
                    printf("Read socket=<%d> len=%d\n",csock,ret);
                
                cell_epoll_ctl(epfd,EPOLL_CTL_MOD,csock,EPOLLOUT);
                    
            }

            // write
            if (events[i].events & EPOLLOUT)
            {
                
                SOCKET csock = events[i].data.fd;
                int ret = writeData(csock);
                if (ret <= 0)
                {
                    clientLeave(csock);
                }
                else
                    printf("write socket=<%d> len=%d\n",csock,ret);
                cell_epoll_ctl(epfd,EPOLL_CTL_MOD,csock,EPOLLIN);
                printf("EPOLLOUT|%d\n",msgCount);
            }
            
            if (101 < msgCount )
            {
                for(SOCKET client : g_clients)
                {
                    cell_epoll_ctl(epfd,EPOLL_CTL_DEL,client,0);
                }
            }
            

            // if (events[i].events & EPOLLERR)
            // {
            //     SOCKET csock = events[i].data.fd;
        	// 	printf("EPOLLERR id=%d socket=%d \n", msgCount, csock);
            // }

            // if (events[i].events & EPOLLHUP)
            // {
            //     SOCKET csock = events[i].data.fd;
        	// 	printf("EPOLLHUP id=%d socket=%d \n", msgCount, csock);
            // }
        }   
        
        
    }

    for(SOCKET client : g_clients)
    {
        close(client);
    }
    

	close(epfd);
    close(sock);	    

    printf("exit.\n");

    return 0;
}