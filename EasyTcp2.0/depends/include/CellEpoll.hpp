#ifndef _CELL_EPOLL_HPP_
#define _CELL_EPOLL_HPP_

#if __linux__

#include <stdio.h>
#include <cstdlib>
#include <unistd.h> //uni std
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define EPOLL_ERROR             (-1)

#define MAX_EVENTS 256

class CellEpoll
{    
public:

~CellEpoll()
{
    destory();
}

int Create(int nMaxEvents = MAX_EVENTS)
{
    // 2.6.8 后 __size 参数无意义 最大连接数取决于硬件条件
    // 最大连接数由 epoll 动态管理.
    // cat /proc/sys/fs/file-max
    if(_epfd > 0)
        destory();
    
    _epfd = epoll_create(nMaxEvents);
    if (EPOLL_ERROR == _epfd)
    {
        perror("epoll_create()");
        return _epfd;
    }
    _pEvents = new epoll_event[nMaxEvents];
    _nMaxEvents = nMaxEvents;
    return _epfd;
    
}

void destory()
{
    if (_epfd > 0)
    {
        close(_epfd);
        _epfd = -1;
    }

    if (_pEvents)
    {
        delete[] _pEvents;
        _pEvents = nullptr;
    }

}

int ctl(int opt, SOCKET csock, uint32_t events)
{
    epoll_event ev;
    ev.events = events;
    ev.data.fd = csock;
    int ret = epoll_ctl(_epfd,opt,csock,&ev);
    if(EPOLL_ERROR == ret)
    {
        perror("epoll_ctl()");\
        return ret;
    }
    return ret;
}

int wait(int time_out)
{
    int ret = epoll_wait(_epfd, _pEvents, _nMaxEvents, time_out/*-1 0 unlimit ms*/);

    if (EPOLL_ERROR == ret)
        perror("epoll_wait()");
    return ret;
}

epoll_event* events()
{
    return _pEvents;
}

private:
epoll_event* _pEvents = nullptr;
int _nMaxEvents = 1;
int _epfd = -1;
};
#endif //!__linux__
#endif