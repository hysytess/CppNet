#ifndef _EasyTcpServer1_8_3_hpp_
#define _EasyTcpServer1_8_3_hpp_

#ifdef _WIN32
#define FD_SETSIZE      2506
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include<stdio.h>
#include<vector>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <algorithm>

#include"MessageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "CELLTask.hpp"

//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240*5
#define SEND_BUFF_SZIE 10240*5
#endif // !RECV_BUFF_SZIE
#define _Cell_THREAD_CONT 4

class ClientSocket
{
private:
	SOCKET _sockfd;
	// 接收消息缓冲区
	char _szMsgBuf[RECV_BUFF_SZIE];
	// 数据尾部位置
	int _lastPos;
	// 发送消息缓冲区
	char _szSendBuf[SEND_BUFF_SZIE];
	int _lastSendPos;
public:
	ClientSocket(SOCKET sock = INVALID_SOCKET)
	{
		_sockfd = sock;
		memset(_szMsgBuf, 0, RECV_BUFF_SZIE);
		_lastPos = 0;

		memset(_szSendBuf, 0, SEND_BUFF_SZIE);
		_lastSendPos = 0;
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}
	char* msgBuf()
	{
		return _szMsgBuf;
	}
	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}

	int SendData(DataHeader* header)
	{
		int ret = SOCKET_ERROR;
		// 发送的数据长度
		int nSendLen = header->dataLength;
		const char* pSendData = (const char*)header;
		while (true)
		{
			if (_lastSendPos + nSendLen >= SEND_BUFF_SZIE)
			{
				// 可拷贝的数据长度
				int nCopyLen = SEND_BUFF_SZIE - _lastSendPos;
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//计算剩余数据长度
				pSendData += nCopyLen;
				//剩余数据长度
				nSendLen -= nSendLen;
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SZIE, 0);
				// 数据尾部清零
				_lastSendPos = 0;
				// 发送 错误/中断
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else
			{
				// 将要发送的数据 拷贝到发送缓冲区尾部
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				//计算数据尾部位置
				_lastSendPos += nSendLen;
				break;
			}
		}
		return ret;
	}
};

class CellServer;
// Delegation [consignor]
// {CellServer -infomation-> INetEvent -infomation-> EasyTcpServer(main work thread)}
class INetEvent
{
public:
	//client leave event.
	virtual void OnLeave(ClientSocket* pClient) = 0;
	// request event
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header) = 0;
	// client join event
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	// receive data event
	virtual void OnNetRecv(ClientSocket* pClient) = 0;
};

// 网络消息发送任务
class CellSendMsg2ClientTask :public CellTask
{
	ClientSocket* _pClient;
	DataHeader* _pHeader;
public:
	CellSendMsg2ClientTask(ClientSocket* pClient, DataHeader* pHeader)
	{
		_pClient = pClient;
		_pHeader = pHeader;
	}

	// 执行任务
	void doTask()
	{
		_pClient->SendData(_pHeader);
		delete _pHeader;
	}
};

// 网络消息接受服务
class CellServer
{
private:
	SOCKET _sock;
	//Client sequen
	std::map<SOCKET, ClientSocket*>_clients;
	//Client sequen buff
	std::vector<ClientSocket*>_clientsBuff;
	std::mutex _mutex;
	std::thread* _pThread;
	INetEvent* _pNetEvent;
	CellTaskServer _taskServer;
public:
	std::atomic_int _recvCount;

	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
	}
	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
		delete _pThread;
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (auto iter : _clients)
			{
				closesocket(iter.second->sockfd());
				delete iter.second;
			}
			closesocket(_sock);
#else
			for (auto iter : _clients)
			{
				closesocket(iter.second->sockfd());
				delete iter.second;
			}
			close(_sock);
#endif
			_clients.clear();
		}
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	fd_set _fdRead_bak;
	// 有客户端加入 或者退出 集合 fd_set(fdRead) 改变
	bool _clients_change;
	SOCKET _maxSock;
	void OnRun()
	{
		_clients_change = true;
		while (isRun())
		{
			if (_clientsBuff.size() > 0)
			{
				std::lock_guard<std::mutex>lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}

			fd_set fdRead;

			FD_ZERO(&fdRead);

			if (_clients_change)
			{
				_clients_change = false;
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					_maxSock < iter.second->sockfd() ? _maxSock = iter.second->sockfd() : _maxSock;
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}

			timeval tv{ 0,0 };
			// 若要在CellServer中处理其他业务则用非阻塞模式
			int ret = (int)select(_maxSock + 1, &fdRead, nullptr, nullptr, &tv);
			if (ret < 0)
			{
				printf("Select done.\n");
				Close();
				return;
			}
			else if (ret == 0)
			{
				continue;
			}

#ifdef _WIN32

			for (int n = 0; n < fdRead.fd_count; n++)
			{
				auto iter = _clients.find(fdRead.fd_array[n]);
				if (iter != _clients.end())
				{
					if (-1 == RecvData(iter->second))
					{
						if (_pNetEvent)
							_pNetEvent->OnLeave(iter->second);
						_clients_change = true;
						_clients.erase(iter->first);
					}
				}
				else
				{
					std::cerr << "Oh,shit! what's error?" << std::endl;
				}
			}

#else
			std::vector<ClientSocket*>temp;
			for (auto iter : _clients)
			{
				if (FD_ISSET(iter.second->sockfd(), &fdRead))
				{
					if (-1 == RecvData(iter.second))
					{
						if (_pNetEvent)
							_pNetEvent->OnLeave(iter.second);
						_clients_change = false;
						temp.push_back(iter.second);
					}
				}
			}
			for (auto pClient : temp)
			{
				_clients.erase(pClient->sockfd());
				delete pClient;
			}
#endif
		}
		return;
	}

	int RecvData(ClientSocket* pClient)
	{
		char* szRecv = pClient->msgBuf() + pClient->getLastPos();
		int nLen = (int)recv(pClient->sockfd(), szRecv, (RECV_BUFF_SZIE)-pClient->getLastPos(), 0);
		// 触发事件 【pClient】 对象
		_pNetEvent->OnNetRecv(pClient);
		if (nLen <= 0)
		{
			return -1;
		}

		//memcpy(pClient->msgBuf() + pClient->getLastPos(), szRecv, nLen);

		pClient->setLastPos(pClient->getLastPos() + nLen);

		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)pClient->msgBuf();

			if (pClient->getLastPos() >= header->dataLength)
			{
				int nSize = pClient->getLastPos() - header->dataLength;
				OnNetMsg(pClient, header);
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				pClient->setLastPos(nSize);
			}
			else
			{
				break;
			}
		}
		return 0;
	}

	void addSendTask(ClientSocket* pClient, DataHeader* header)
	{
		CellSendMsg2ClientTask* task = new CellSendMsg2ClientTask(pClient, header);
		_taskServer.addTask(task);
	}

	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		// 触发网络事件.
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

	void addClient(ClientSocket* pClient)
	{
		std::lock_guard<std::mutex>lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void CellsrvStart()
	{
		_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
		_taskServer.Start();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
};

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

	virtual void OnNetMsg(CellServer* pCellServer, SOCKET cSock, DataHeader* header)
	{
		_msgCount++;
	}

	virtual void OnNetRecv(ClientSocket* pClient)
	{
		_recvCount++;
	}
};
//应用层
class MyServer :public EasyTcpServer
{
private:
	std::map<SOCKET, ClientSocket*>_clientMap;
public:
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
		_clientMap.insert(std::pair<SOCKET, ClientSocket*>(pClient->sockfd(), pClient));
		//printf("client<%d> join.\n", pClient->sockfd());
	}

	virtual void OnNetLeave(ClientSocket* pClient)
	{
		EasyTcpServer::OnLeave(pClient);
		if (_clientMap.count(pClient->sockfd()))
			_clientMap.erase(pClient->sockfd());
		//printf("client<%d> leave.\n", (int)pClient->sockfd());
	}

	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pCellServer, pClient->sockfd(), header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login = (Login*)header;
			//printf("收到客户端<socket=%d>请求：CMD_LOGIN，数据长度：%d, userName=%s passWord=%s", (int)cSock, login->dataLength, login->userName, login->PassWord);
			//LoginResult ret;
			//SendData(pClient, &ret);
			// 不安全写法 将业务层暴露给用户(应用层)
			LoginResult *ret = new LoginResult();

			pCellServer->addSendTask(pClient, ret);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("收到客户端<socket=%d>请求：CMD_LOGOUT，数据长度：%d, userName=%s", (int)cSock, login->dataLength, login->userName);
			LogoutResult ret;
			SendData(pClient, &ret);
		}
		break;

		default:
		{
			printf("<socket=%d>Error infomation, dataLength: %d\n", (int)pClient->sockfd(), header->dataLength);
		}
		break;
		}
	}

	int SendData(ClientSocket* pClient, DataHeader* header)
	{
		if (pClient->sockfd() && header)
		{
			return send(pClient->sockfd(), (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	void SendDataToAll(DataHeader* header)
	{
		for (auto _clientMap_iter = _clientMap.begin(); _clientMap_iter != _clientMap.end(); _clientMap_iter++)
		{
			SendData(_clientMap_iter->second, header);
		}
	}
};

#endif