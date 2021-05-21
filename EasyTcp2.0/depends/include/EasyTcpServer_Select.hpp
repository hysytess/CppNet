#ifndef _EasyTcpServer_Select_hpp_
#define _EasyTcpServer_Select_hpp_

#include "CellServer_Select.hpp"
#include "EasyTcpServer2_4.hpp"


class EasyTcpServer_Select :public EasyTcpServer
{
public:
	void Start(int nCellServer)
	{
		EasyTcpServer::Start<CellServer_Select>(nCellServer);
	}
protected:
	void OnRun(CellThread* pThread)
	{
		CellFDSet fdRead;
		while (pThread->isRun())
		{
			Time4msg();

			fdRead.zero();

			fdRead.add(sockfd());

			timeval tv{ 0,1 };
			int ret = (int)select(sockfd() + 1, fdRead.fdset(), nullptr, nullptr, &tv);

			if (ret < 0)
			{
				CellLog_Debug("EasyTcpServer.accept.select exit.");
				pThread->Exit();
				break;
			}

			if (fdRead.has(sockfd()))
			{
				//fdRead.del(_sock);
				Accept();
			}
		}
	}

};

#endif