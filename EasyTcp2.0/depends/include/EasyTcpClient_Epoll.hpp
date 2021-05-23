#ifndef _EasyTcpClient_Epoll_hpp_
#define _EasyTcpClient_Epoll_hpp_
#if __linux__

#include "EasyTcpClient2_4.hpp"
#include "CellEpoll.hpp"

class EasyTcpClient_Epoll:public EasyTcpClient
{
public:
	virtual void OnInitSocket() 
	{
		_ep.Create(1);
		_ep.ctl(EPOLL_CTL_ADD,_pClient,EPOLLIN);
	}

	void Close()
	{
		_ep.destory();
		EasyTcpClient::Close();
	}
	bool OnRun(int microseconds = 1)
	{
		if (isRun())
		{

			if (_pClient->needWrite())
			{
				_ep.ctl(EPOLL_CTL_MOD,_pClient,EPOLLIN|EPOLLOUT);
			}
			else
			{
				_ep.ctl(EPOLL_CTL_MOD,_pClient,EPOLLIN);
			}
			
			int ret = _ep.wait(microseconds);
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
						if (SOCKET_ERROR == RecvData())
						{
							CellLog_PError("<socket=%d>OnRun.epoll RecvData exit",(int)pClient->sockfd());
							Close();							
							continue;
						}
					}
					if(events[i].events & EPOLLOUT)
        			{
						if (SOCKET_ERROR == pClient->SendDataReal())
						{
							CellLog_PError("<socket=%d>OnRun.epoll SendDataReal exit",(int)pClient->sockfd());
							Close();
						}
					}
				}
			}
			return true;
		}
		return false;
	}



private:
	CellEpoll _ep;
};

#endif //!__linux__
#endif