// 加入多线程控制台
// 兼容多平台

#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <thread>

#ifdef _WIN32 // WinSocket ��̬��
#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define  _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR           (-1)
#endif

#include "MSGPROC.hpp"

using namespace std;

bool g_isrun = true;

int processor(SOCKET sock);
void broadcast(SOCKET _cSock);
void Getcmd();

int main(int argc, char** argv) {
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	int err = WSAStartup(ver, &dat); // ����Windows ���绷��
	if (err != 0)
	{
		return 1;
	}
	if (LOBYTE(dat.wVersion) != 2 || HIBYTE(dat.wVersion) != 2)
	{
		WSACleanup();
		return 1;
	}
#endif
	// frist build a socket
	// ipv4/6 stream tcp/udp
	SOCKET _ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// the second binding port with ip adress;
	sockaddr_in _sin{};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(5566); // host to net unsigned short
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1"); // INADDR_ANY;
#else
	_sin.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1"); // INADDR_ANY;
#endif
	// bind() �������ж϶˿��Ƿ�ռ��
	int ret = ::bind(_ServerSocket, (sockaddr *)&_sin, sizeof(_sin));
	if (SOCKET_ERROR == ret)
	{
#ifdef _WIN32
		cout << "Bind error! Error code:" << WSAGetLastError() << endl;
		closesocket(_ServerSocket);
		WSACleanup();
#else
		cout << "Bind error!" << endl;
		close(_ServerSocket);
#endif
		return 1;
	}
	// the third listen
	if (SOCKET_ERROR == listen(_ServerSocket, 5)) // �����׽��� ����������
	{
		cout << "Listen error!" << endl;
#ifdef _WIN32
		closesocket(_ServerSocket);
		WSACleanup();
#else
		close(_ServerSocket);
#endif
		return 1;
	}
	else
	{
		cout << "Starup server..." << endl
			<< "Wait for client connect...port:5566 " << endl;
	}

	thread t1(Getcmd);
	t1.detach();

	while (g_isrun) {
		// Select Model:
		// the frist Create and Init set.
		fd_set readfds; // Create read-Socket
		fd_set writefds;// Create write-Socket [�˴���ʱ�����õ�]
		fd_set exceptfds;// Create exception-Socket [error socket set �˴���ʱ�����õ�]

		FD_ZERO(&readfds); // Init -> clear
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		// the Seconde put Server's socket[listen-socket] into read-set��write-set��exception-set
		FD_SET(_ServerSocket, &readfds);
		FD_SET(_ServerSocket, &writefds);
		FD_SET(_ServerSocket, &exceptfds);

		// the third put new client's socket into read-set/write-set/exception-set for select.
		SOCKET maxSock = _ServerSocket;
		for (int i = (int)g_clientList.size() - 1; i >= 0; i--) {
			FD_SET(g_clientList[i], &readfds);
			maxSock < g_clientList[i] ? maxSock = g_clientList[i] : maxSock;
		}

		// fourth setting timeout
		timeval tv{ 0,0 };

		// fifth Starup Select engine
		int ret = (int)select(maxSock + 1, &readfds, &writefds, &exceptfds, &tv);
		if (ret < 0) // Select work done. No Work,No client - no client in read-set/write-set/exception-set and g_clientList is empty.
		{
#ifdef _WIN32
			cout << "Select done. terminal code: " << WSAGetLastError() << endl;
#else
			cout << "Select done. " << endl;
#endif
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
#ifdef _WIN32
			_cSock = accept(_ServerSocket, (sockaddr*)&_clientAddr, &nAdddrLen);
#else
			_cSock = accept(_ServerSocket, (sockaddr*)&_clientAddr, (socklen_t*)&nAdddrLen);
#endif
			if (_cSock == INVALID_SOCKET) {
#ifdef _WIN32
				cout << "Error! accept Invalid socket. error code: " << WSAGetLastError() << endl;
#else
				cout << "Error! accept Invalid socket." << endl;
#endif
			}
			else
			{
				g_clientList.push_back(_cSock);
				cout << inet_ntoa(_clientAddr.sin_addr) << "<" << (int)_cSock << ">join." << endl;
				broadcast(_cSock);
			}
		}
		// deal request [some had connective client send request(data) to server]
		// if server need to send data to client,jump to write branch -->
		// wirte brand:send data to some client,server's socket[listen socket] will write data [read event]
		// ѭ������һ���Ѿ��ڿɶ�����[read-set]�еĿͻ���socket

		for (int n = (int)g_clientList.size() - 1; n >= 0; n--) {
			if (FD_ISSET(g_clientList[n], &readfds)) {
				if (-1 == processor(g_clientList[n])) {
					auto g_clientList_iter = g_clientList.begin() + n;
					if (g_clientList_iter != g_clientList.end())
					{
						g_clientList.erase(g_clientList_iter);
					}
				}
			}
		}

		// Select task space...deal some thing
		//cout << "Select task space...deal some business." << endl;
	}

	// sixth close socket
	for (int n = (int)g_clientList.size() - 1; n >= 0; n--)
	{
#ifdef _WIN32
		closesocket(g_clientList[n]);
#else
		close(g_clientList[n]);
#endif
	}

#ifdef _WIN32
	closesocket(_ServerSocket);
	WSACleanup();
	system("pause");
#else
	close(_ServerSocket);
#endif
	return 0;
}

int processor(SOCKET _cSock) {
	// fifth receive-send and deal request
	char szRecv[4096]{};
	int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		cout << "Client <" << (int)_cSock << ">was exit!" << endl;
		return -1; // client was exit/death.
	}
	//cout << "From Client<" << (int)_cSock << "> request: " << header->cmd
	//	<< " DataLength: " << header->dataLength << endl;
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
		//cout << "From Client<" << (int)_cSock << "> message: " << facebackmsg.msg << endl;
	}
	break;
	case DPT_EXIT:
	{
		cout << "From Client<" << (int)_cSock << "> exit." << endl;
	}
	break;
	default:
		cout << "<socket= " << (int)_cSock << ">Error. cmd: " << header->cmd
			<< " dataLength: " << header->dataLength << endl;
		break;
	}
	return 0;
}
void broadcast(SOCKET _cSock) {
	NewUserJoinInfo broadcastInfo{};
	broadcastInfo.socketId = (int)_cSock;
	for (SOCKET s : g_clientList)
		send(s, (char*)&broadcastInfo, sizeof(NewUserJoinInfo), 0);
}

void Getcmd() {
	char cmdbuf[128]{};
	while (true) {
		cin >> cmdbuf;
		if (!strcmp(cmdbuf, "exit"))
		{
			g_isrun = false;
			break;
		}
	}
}