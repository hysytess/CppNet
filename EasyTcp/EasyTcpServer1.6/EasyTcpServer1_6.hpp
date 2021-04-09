#ifndef _EasyTcpServer1_6_hpp_
#define _EasyTcpServer1_6_hpp_

#ifdef _WIN32
#define FD_SETSIZE      1024
#define WIN32_LEAN_AND_MEAN
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
#include"MessageHeader.hpp"

//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE

class ClientSocket
{
public:
	ClientSocket(SOCKET sock = INVALID_SOCKET)
	{
		_sockfd = sock;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
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
private:
	SOCKET _sockfd;
	char _szMsgBuf[RECV_BUFF_SZIE * 10];
	int _lastPos;
};

class EasyTcpServer {
private:
	SOCKET _sock;
	std::vector<ClientSocket*>_clients;
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
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
			_clients.push_back(new ClientSocket(csock));
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);
			printf("New client socket=<%d> join, socket = %d, IP = %s Count<%d>\n", (int)_sock, (int)csock, inet_ntoa(clientAddr.sin_addr), (int)_clients.size());
		}
		return csock;
	}

	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			closesocket(_sock);
			WSACleanup();
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];
			}
			close(_sock);
			_clients.clear();
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
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;

			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);

			SOCKET maxSock = _sock;
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);
				maxSock < _clients[n]->sockfd() ? maxSock = _clients[n]->sockfd() : maxSock;
			}

			timeval tv{ 1,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &tv);

			if (ret < 0)
			{
				printf("Select done.\n");
				Close();
				return false;
			}

			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{
					if (-1 == RecvData(_clients[n]))
					{
						auto _clients_iter = _clients.begin() + n;
						if (_clients_iter != _clients.end())
						{
							delete _clients[n];
							_clients.erase(_clients_iter);
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	char _szRecv[RECV_BUFF_SZIE]{};
	int RecvData(ClientSocket* pClient)
	{
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SZIE, 0);
		if (nLen <= 0)
		{
			printf("Client <socket=%d> was exit. Count<%d>\n", pClient->sockfd(), (int)_clients.size());
			return -1;
		}

		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);

		pClient->setLastPos(pClient->getLastPos() + nLen);

		while (pClient->getLastPos() >= sizeof(DataHeader))
		{
			DataHeader* header = (DataHeader*)pClient->msgBuf();

			if (pClient->getLastPos() >= header->dataLength)
			{
				int nSize = pClient->getLastPos() - header->dataLength;
				OnNetMsg(pClient->sockfd(), header);
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

	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	void SendDataToAll(DataHeader* header)
	{
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
			SendData(_clients[n]->sockfd(), header);
	}

	virtual void OnNetMsg(SOCKET cSock, DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login *login = (Login*)header;
			//printf("收到客户端<socket=%d>请求：CMD_LOGIN，数据长度：%d, userName=%s passWord=%s", (int)cSock, login->dataLength, login->userName, login->PassWord);
			LoginResult ret;
			SendData(cSock, &ret);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("收到客户端<socket=%d>请求：CMD_LOGOUT，数据长度：%d, userName=%s", (int)cSock, login->dataLength, login->userName);
			LogoutResult ret;
			SendData(cSock, &ret);
		}
		break;

		default:
		{
			printf("<socket=%d>Error infomation, dataLength: %d\n", (int)cSock, header->dataLength);
		}
		break;
		}
	}
};

#endif