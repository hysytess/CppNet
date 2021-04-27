#ifndef _EasyTcpServer1_8_4_hpp_
#define _EasyTcpServer1_8_4_hpp_

#include "PublicLib.hpp"

#include <thread>
#include <mutex>
#include <atomic>

#include "CELLTimestamp.hpp"
#include "CELLTask.hpp"
#include "ClientSocket.hpp"
#include "INETEVENT.hpp"
#include "CellServer.hpp"

class EasyTcpServer :public INetEvent
{
private:
	SOCKET _sock;
	// 消息处理对象 初始化启动线程
	std::vector<CellServer*> _cellServers;
	CELLTimestamp _tTime;

protected:
	std::atomic_int _recvCount;
	std::atomic_int _msgCount;
	std::atomic_int _clientCount;

public:

	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_clientCount = 0;
		_msgCount = 0;
	}
	virtual ~EasyTcpServer()
	{
		Close();
	}

	SOCKET InitSocket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>old connection was disconneted.\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("Error, Create socket fail...\n");
		}
		else
		{
			printf("<socket=%d> was created...\n", (int)_sock);
		}
		return _sock;
	}

	int Bind(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}
		sockaddr_in _sin{};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);

#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret)
		{
			printf("Error, bind port<%d> fail...\n", port);
		}
		else
		{
			printf("Port<%d> bind sucess...\n", port);
		}
		return ret;
	}

	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (ret == SOCKET_ERROR)
		{
			printf("<socket=%d> listen error...\n", (int)_sock);
		}
		else
		{
			printf("<socket=%d> listen, wait for client connect...\n", (int)_sock);
		}
		return ret;
	}

	SOCKET Accept()
	{
		sockaddr_in clientAddr{};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET csock = INVALID_SOCKET;
#ifdef _WIN32
		csock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		csock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);

#endif
		if (INVALID_SOCKET == csock)
		{
			printf("socket=<%d> error, invalid SOCKET...\n", (int)csock);
		}
		else
		{
			addClientToCellServer(new ClientSocket(csock));
			//client ip: inet_ntoa(clientAddr.sin_addr);
		}
		return csock;
	}

	void addClientToCellServer(ClientSocket* pClient)
	{
		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
		OnNetJoin(pClient);
	}

	void Start(int nCellServer)
	{
		for (int n = 0; n < nCellServer; n++)
		{
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			// 注册网络事件接受对象
			ser->setEventObj(this);
			// 启动消息线程
			ser->CellsrvStart();
		}
	}

	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			WSACleanup();
#else
			close(_sock);
#endif
		}
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	bool OnRun()
	{
		if (isRun())
		{
			Time4msg();

			fd_set fdRead;

			FD_ZERO(&fdRead);

			FD_SET(_sock, &fdRead);

			timeval tv{ 0,10 };
			int ret = (int)select(_sock + 1, &fdRead, nullptr, nullptr, &tv);

			if (ret < 0)
			{
				printf("Accept::select done.\n");
				Close();
				return false;
			}

			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
				return true;
			}
			return true;
		}
		return false;
	}

	void Time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		if (t1 >= 1.0)
		{
			printf("thread<%d>,time<%lf>,listen<%d>,client<%d>,receive<%d>,msg<%d>\n", (int)_cellServers.size(), t1, (int)_sock, (int)_clientCount, (int)(_recvCount / t1), (int)(_msgCount / t1));
			_recvCount = 0;
			_msgCount = 0;
			_tTime.update();
		}
	}

	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
	}

	virtual void OnLeave(ClientSocket* pClient)
	{
		_clientCount--;
	}

	virtual void OnNetMsg(CellServer* pCellServer, SOCKET cSock, netmsg_DataHeader* header)
	{
		_msgCount++;
	}

	virtual void OnNetRecv(ClientSocket* pClient)
	{
		_recvCount++;
	}
};

#endif