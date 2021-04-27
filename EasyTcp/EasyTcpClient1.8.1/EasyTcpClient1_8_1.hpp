#ifndef _EasyTcpClient1_8_1_hpp_
#define _EasyTcpClient1_8_1_hpp_

#ifdef _WIN32
#define FD_SETSIZE      2506
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#pragma  comment(lib,"ws2_32.lib")

#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
#include <stdio.h>
#include <iostream>
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

class EasyTcpClient
{
	SOCKET _sock;
	bool _isConnected;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
		_isConnected = false;
	}

	virtual ~EasyTcpClient()
	{
		Close();
	}

	void InitSocket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket=%d>old connection was disconnected...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("Error, create socket<%d> fail...\n", (int)_sock);
		}
		else
		{
			//printf("Create socket=<%d> sucess...\n", (int)_sock);
		}
	}

	int Connect(const char* ip, unsigned short port)
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
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif // _WIN32
		//printf("<socket=%d> connect to host<%s:%d>...\n", (int)_sock, ip, port);
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d> error, connect to host<%s:%d> fail...\n", (int)_sock, ip, port);
		}
		else
		{
			_isConnected = true;
			//printf("<socket=%d> connect to host<%s:%d> sucess.\n", (int)_sock, ip, port);
		}
		return ret;
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
			_sock = INVALID_SOCKET;
		}
		_isConnected = false;
	}

	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval tv = { 0,0 };
			int ret = (int)select(_sock + 1, &fdReads, 0, 0, &tv);
			if (ret < 0)
			{
				printf("<socket=%d>select done. terminal code:1\n", (int)_sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select done. terminal code:2\n", (int)_sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	bool isRun()
	{
		return (_sock != INVALID_SOCKET) && _isConnected;
	}

	char _szMsgBuf[RECV_BUFF_SIZE * 10]{};
	char _szRecv[RECV_BUFF_SIZE]{};
	int _lastPos = 0;

	int RecvData(SOCKET csock)
	{
		int nLen = (int)recv(csock, _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("<socket=%d>disconneted.\n", (int)csock);
			return -1;
		}
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		_lastPos += nLen;
		while (_lastPos >= sizeof(DataHeader))
		{
			DataHeader *header = (DataHeader*)_szMsgBuf;
			if (_lastPos >= header->dataLength)
			{
				int nSize = _lastPos - header->dataLength;
				OnNetMsg(header);
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, nSize);
				_lastPos = nSize;
			}
			else
			{
				break;
			}
		}
		return 0;
	}
	virtual void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* login = (LoginResult*)header;
			//printf("<socket=%d>收到服务器消息：CMD_LOGIN_RESULT,数据长度：%d\n", _sock, header->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logout = (LogoutResult*)header;
			//printf("<socket=%d>收到服务器消息：CMD_LOGOUT_RESULT,数据长度：%d\n", _sock, header->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			//printf("<socket=%d>收到服务器消息：CMD_NEW_USER_JOIN,数据长度：%d\n", _sock, header->dataLength);
		}
		break;
		case CMD_ERROR:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			//printf("<socket=%d>收到服务器消息：CMD_ERROR,数据长度：%d\n", _sock, header->dataLength);
		}
		break;
		default:
		{
			printf("Error! <socket=%d>, dataLength: %d\n", (int)_sock, header->dataLength);
		}
		}
	}
	int SendData(DataHeader* header, int nLen)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, nLen, 0);
		}
		return SOCKET_ERROR;
	}
};

#endif