#include <iostream>
#include <cstdlib>
#include <algorithm>

#define  _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <WinSock2.h>

#ifndef _WIN32 // WinSocket 动态库
#pragma comment(lib,"ws2_32.lib")
#endif

#include "MSGPROC.hpp"

using namespace std;

int processor(SOCKET sock);

int main(int argc, char** argv) {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	int err = WSAStartup(ver, &dat); // 启动Windows 网络环境
	if (err != 0)
	{
		return 1;
	}
	if (LOBYTE(dat.wVersion) != 2 || HIBYTE(dat.wVersion) != 2)
	{
		WSACleanup();
		return 1;
	}

	// frist build a socket
	// ipv4/6 stream tcp/udp
	SOCKET _ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// the second binding port with ip adress;
	sockaddr_in _sin{};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(233); // host to net unsigned short
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // INADDR_ANY
	// bind() 可用来判断端口是否被占用
	if (SOCKET_ERROR == ::bind(_ServerSocket, (sockaddr *)&_sin, sizeof(_sin))) {
		cout << "Bind error! Error code:" << WSAGetLastError() << endl;
		closesocket(_ServerSocket);
		WSACleanup();
		return 1;
	}
	else
		cout << "Bind success!" << endl;

	// the third listen
	if (SOCKET_ERROR == listen(_ServerSocket, 5)) // 监听套接字 最大监听队列
	{
		cout << "Listen error!" << endl;
		closesocket(_ServerSocket);
		WSACleanup();
		return 1;
	}
	else
		cout << "Wait for client connect......" << endl;

	while (true) {
		// Select Model:
		// the frist Create and Init set.
		fd_set readfds; // Create read-Socket
		fd_set writefds;// Create write-Socket
		fd_set exceptfds;// Create exception-Socket [error socket set]

		FD_ZERO(&readfds); // Init -> clear
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		// the Seconde put Server's socket[listen-socket] into read-set、write-set、exception-set
		FD_SET(_ServerSocket, &readfds);
		FD_SET(_ServerSocket, &writefds);
		FD_SET(_ServerSocket, &exceptfds);

		// the third put client's socket into read-set.
		for (int i = (int)g_clientList.size() - 1; i >= 0; i--)
			FD_SET(g_clientList[i], &readfds);

		// fourth setting timeout
		timeval tv{ 1,0 };

		// fifth Starup Select engine
		int ret = (int)select(_ServerSocket + 1, &readfds, &writefds, &exceptfds, &tv);
		if (ret < 0) // Select work done. No Work,No client - no client in read-set/write-set/exception-set and g_clientList is empty.
		{
			cout << "Select done. terminal code: " << WSAGetLastError() << endl;
			break;
		}

		// read branch:some client send data,server(server's socket[listen socket] had readly data) will to read data [read event]
		if (FD_ISSET(_ServerSocket, &readfds))
		{
			FD_CLR(_ServerSocket, &readfds);
			// if it's a new client join,accept this connect's request, and put this client's socket into g_clientList
			// fourth accept request
			sockaddr_in _clientAddr{};
			int nAdddrLen = sizeof(sockaddr_in);
			SOCKET _cSock = INVALID_SOCKET;

			_cSock = accept(_ServerSocket, (sockaddr*)&_clientAddr, &nAdddrLen);
			if (_cSock == INVALID_SOCKET)
				cout << "Error! accept Invalid socket. error code: " << WSAGetLastError() << endl;
			else
			{
				g_clientList.push_back(_cSock);
				cout << inet_ntoa(_clientAddr.sin_addr) << "<" << (int)_cSock << ">join." << endl;
			}
		}
		// deal request [some had connective client send request(data) to server]
		// if server need to send data to client,jump to write branch -->
		// wirte brand:send data to some client,server's socket[listen socket] will write data [read event]
		for (int k = 0; k < (int)readfds.fd_count; ++k) {
			// deal client's request
			// client was exit or death, throw out this client from g_clientList.
			if (-1 == processor(readfds.fd_array[k]))
			{
				auto g_clientList_iter = std::find(g_clientList.begin(), g_clientList.end(), readfds.fd_array[k]);
				if (g_clientList_iter != g_clientList.end())
					g_clientList.erase(g_clientList_iter);
			}
		}

		// Select task space...deal some thing
		//cout << "Select task space...deal some business." << endl;
	}

	// sixth close socket
	for (int n = (int)g_clientList.size() - 1; n >= 0; n--)
		closesocket(g_clientList[n]);
	closesocket(_ServerSocket);
	WSACleanup();
	system("pause");
	return 0;
}

int processor(SOCKET _cSock) {
	// fifth receive-send and deal request
	char szRecv[1024]{};
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		cout << "Client <" << (int)_cSock << ">was exit!" << endl;
		return -1; // client was exit/death.
	}
	cout << "From Client<" << (int)_cSock << "> request: " << header->cmd
		<< " DataLength: " << header->dataLength << endl;

	switch (header->cmd)
	{
	case DPT_LOGIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login* login = (Login*)szRecv;
		// login sucess!
		if (!strcmp("User123", login->UserName) && !strcmp("password123", login->PassWord))
		{
			LoginResult ret{};
			ret.cmd = DPT_LOGINOK;
			ret.result = DPT_LOGINOK;
			send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
		}
		else // login fail!
		{
			LoginResult ret{};
			ret.cmd = DPT_LOGINERROR;
			ret.result = DPT_LOGINERROR;
			send(_cSock, (char *)&ret, sizeof(LoginResult), 0);
		}
	}
	break;
	case DPT_LOGOUT:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Loginout* loginout = (Loginout*)szRecv;
		// loginout sucess!
		if (!strcmp("User123", loginout->UserName))
		{
			LoginoutResult ret{};
			ret.result = DPT_LOGOUTOK;
			send(_cSock, (char *)&ret, sizeof(LoginoutResult), 0);
			cout << loginout->UserName << " loginout." << endl;
		}
		else // loginout fail!
		{
			LoginoutResult ret{};
			ret.cmd = DPT_LOGOUTERROR;
			ret.result = DPT_LOGOUTERROR;
			send(_cSock, (char *)&ret, sizeof(LoginoutResult), 0);
		}
	}
	break;
	case DPT_MSG:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Message* msgbuf = (Message*)szRecv;
		Message facebackmsg;
		strcpy(facebackmsg.msg, msgbuf->msg);
		send(_cSock, (char *)&facebackmsg, sizeof(Message), 0);
		cout << "From Client<" << (int)_cSock << "> message: " << facebackmsg.msg << endl;
	}
	break;
	case DPT_EXIT:
	{
		cout << "From Client<" << (int)_cSock << "> exit." << endl;
	}
	break;
	default:
		break;
	}
}