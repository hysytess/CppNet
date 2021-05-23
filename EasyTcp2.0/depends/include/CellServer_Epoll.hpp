#ifndef _CellServer_Epoll_HPP_
#define _CellServer_Epoll_HPP_
#if __linux__

#include "CellServer_2.hpp"
#include "CellEpoll.hpp"

// 网络消息接受服务
class CellServer_Epoll:public CellServer
{

public:

	CellServer_Epoll()
	{
		_ep.Create(10240);
	}

	~CellServer_Epoll()
	{
		Close();
	}

	bool DoNetEvent()
	{
		for (auto iter : _clients)
		{
			if (iter.second->needWrite())
			{
				_ep.ctl(EPOLL_CTL_MOD,iter.second,EPOLLIN|EPOLLOUT);
			}
			else
			{
				_ep.ctl(EPOLL_CTL_MOD,iter.second,EPOLLIN);
			}
		}

		int ret = _ep.wait(1);
		if(ret < 0)
		{
			if (errno == EINTR)
				return true;
			
			perror("CellServer_Epoll.OnRun.Wait Eorr exit");
			return false;

		}
		else if(ret == 0)
			return true;

		auto events = _ep.events();
        for(int i = 0;i < ret;i++)
        {
			ClientSocket* pClient = (ClientSocket*)events[i].data.ptr;
        	if (pClient)
        	{
            // read
            	if(events[i].events & EPOLLIN)
            	{
					if (SOCKET_ERROR == RecvData(pClient))
					{
						rmClient(pClient);
						continue;
					}
				}
				if(events[i].events & EPOLLOUT)
            	{
					if (SOCKET_ERROR == pClient->SendDataReal())
					{
						rmClient(pClient);
					}
				}
			}
		}

		return true; 
	}

	void rmClient(ClientSocket* pClient)
	{
		auto iter = _clients.find(pClient->sockfd());
		if (iter != _clients.end())
			_clients.erase(iter);
		
		onClientLeave(pClient);

	}

	void onClientJoin(ClientSocket* pClient)
	{
		// _ep.ctl(EPOLL_CTL_ADD,pClient->sockfd(),EPOLLIN);
		_ep.ctl(EPOLL_CTL_ADD,pClient,EPOLLIN);
	}


private:
	CellEpoll _ep;
};

#endif // !__linux__
#endif