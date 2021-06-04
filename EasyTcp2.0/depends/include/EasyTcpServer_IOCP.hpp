#ifndef _EasyTcpServer_IOCP_hpp_
#define _EasyTcpServer_IOCP_hpp_
#ifdef _WIN32

#ifndef CELL_USE_IOCP
#define CELL_USE_IOCP
#endif

#include "CellServer_IOCP.hpp"
#include "EasyTcpServer2_4.hpp"
#include "CellIOCP.hpp"

class EasyTcpServer_IOCP :public EasyTcpServer
{
public:
	void Start(int nCellServer)
	{
		EasyTcpServer::Start<CellServer_IOCP>(nCellServer);
	}
protected:
	void OnRun(CellThread* pThread)
	{
		CellIOCP iocp;
		iocp.Create();
		iocp.reg(sockfd());
		iocp.loadAcceptEx(sockfd());

		//const int len = 2 * (sizeof(sockaddr_in) + 16);
		const int len = 1024;
		char buf[len] = {};

		IO_DATA_BASE ioData{};
		ioData.wsBuff.buf = buf;
		ioData.wsBuff.len = len;
		
		iocp.postAccept(&ioData);

		IOCP_EVENT ioEvent = {};
		while (pThread->isRun())
		{
			Time4msg();

			int retwait = iocp.wait(ioEvent,1);
			if (retwait < 0)
			{
				CellLog_Debug("EasyTcpServer.accept.epoll exit.");
				pThread->Exit();
				break;
			}
			if (0 == retwait)
				continue;

			if (IO_TYPE::TYPE_ACCEPT == ioEvent.pIOData->iotype)
			{
				iocp.postAccept(&ioData);
				IOCP_Accept(ioEvent.pIOData->sockfd);
				//CellLog_Debug("New client<socket=%d> join.\n", (int)ioEvent.pIOData->sockfd);
			}

		}	
	}

	SOCKET IOCP_Accept(SOCKET csock)
	{
		//sockaddr_in clientAddr{};
		//int nAddrLen = sizeof(sockaddr_in);
		//SOCKET csock = INVALID_SOCKET;

		if (INVALID_SOCKET == csock)
		{
			CellLog_Debug("socket=<%d> error, invalid SOCKET...error code:%d %s.", (int)csock, errno, strerror(errno));
		}
		else
		{
			if (_clientCount < _nMaxClient)
			{
				addClientToCellServer(new ClientSocket(csock, _nSendBuffSize, _nRecvBuffSize));
			}
			else
			{
				CellNetWork::destorySocket(csock);
				CellLog_Warring("Accept to nMaxClient");
			}
			//client ip: inet_ntoa(clientAddr.sin_addr);
		}
		return csock;
	}
};

#endif //!__linux__
#endif