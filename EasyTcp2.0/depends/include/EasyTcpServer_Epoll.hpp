#ifndef _EasyTcpServer_Epoll_hpp_
#define _EasyTcpServer_Epoll_hpp_
#if __linux__

#include "CellServer_Epoll.hpp"
#include "EasyTcpServer2_4.hpp"
#include "CellEpoll.hpp"


class EasyTcpServer_Epoll :public EasyTcpServer
{
public:
	void Start(int nCellServer)
	{
		EasyTcpServer::Start<CellServer_Epoll>(nCellServer);
	}
protected:
	void OnRun(CellThread* pThread)
	{
		CellEpoll ep;
		ep.Create(1);
		ep.ctl(EPOLL_CTL_ADD,sockfd(),EPOLLIN);
		while (pThread->isRun())
		{
			Time4msg();

			int ret = ep.wait(1);
			if (ret < 0)
			{
				CellLog_Debug("EasyTcpServer.accept.epoll exit.");
				pThread->Exit();
				break;
			}

			auto events = ep.events();
        	for(int i = 0;i < ret;i++)
        	{
            	if (events[i].data.fd == sockfd())
            	{
                // read
                	if(events[i].events & EPOLLIN)
                	{
  						Accept();
					}
				}
			}
		}	
	}

};

#endif //!__linux__
#endif