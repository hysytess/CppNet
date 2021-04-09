#ifndef _EasyTcpServer1_5_hpp
#define _EasyTcpServer1_5_hpp
#include "MSGPROC.hpp"

#include <iostream>
#include <cstdlib>
#include <thread>
#include <vector>

#ifdef _WIN32

#define  _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define strcpy strcpy_s
#pragma comment(lib,"ws2_32.lib")

#include <windows.h>
#include <WinSock2.h>

#else

#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR   (SOCKET)(-1)
#endif

using namespace std;

class EasyTcpServer
{
public:
	EasyTcpServer();
	virtual ~EasyTcpServer();
#ifdef _WIN32
	int InitNetwork(int hibyte, int lobyte);
#endif
	SOCKET InitSocket();
	int Bind(const char* ip, unsigned short port);
	int Listen(int n);
	SOCKET Accept();
	bool OnRun();
	bool isRun();
	int RecvData(SOCKET _cSock);
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header);
	int sendToClient(SOCKET _cSock, DataHeader *dataPack);
	void broadcast(SOCKET _cSock);
	SOCKET getSock();
	void CloseScok();
protected:
private:
	SOCKET _ServerSocket;
	vector<SOCKET> g_clientList;
};

EasyTcpServer::EasyTcpServer()
{
#ifdef _WIN32
	//!0 InitNetwork fail.
	if (!InitNetwork(2, 2))
	{
		cerr << "Init network fail!\n";
		exit(1);
	}
#endif
	_ServerSocket = INVALID_SOCKET;
}
EasyTcpServer::~EasyTcpServer()
{
	CloseScok();
}

#ifdef _WIN32
// startup return 1 else 0
int EasyTcpServer::InitNetwork(int hibyte, int lobyte)
{
	WORD ver = MAKEWORD(hibyte, lobyte);
	WSADATA dat;
	WSAStartup(ver, &dat);
	int err = WSAStartup(ver, &dat);
	if (err != 0)
	{
		return 0;
	}
	if (LOBYTE(dat.wVersion) != 2 || HIBYTE(dat.wVersion) != 2)
	{
		WSACleanup();
		return 0;
	}
	return 1;
}
#endif

SOCKET EasyTcpServer::InitSocket()
{
	// seconde build a socket
	// ipv4/6 stream tcp/udp
	if (_ServerSocket != INVALID_SOCKET)
	{
		CloseScok();
		cerr << "<socket= " << (int)getSock() << ">disconnect old connection.\n";
	}

	// frist build a socket
	// ipv4/6 stream tcp/udp
	_ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _ServerSocket)
	{
#ifdef _WIN32
		cerr << "Build socket fail! error code:" << WSAGetLastError() << endl;
		closesocket(_ServerSocket);
		WSACleanup();
#else
		cerr << "Build socket fail!" << endl;
		close(_ServerSocket);
#endif
		return -1;
	}
	return _ServerSocket;
}

// bind ip and port
int EasyTcpServer::Bind(const char* ip, unsigned short port)
{
	// the second binding port with ip adress;
	sockaddr_in _sin{};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port); // host to net unsigned short
#ifdef _WIN32
	if (ip)
		_sin.sin_addr.S_un.S_addr = inet_addr(ip); //inet_addr("127.0.0.1"); // INADDR_ANY;
	else
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
	if (ip)
		_sin.sin_addr.s_addr = inet_addr(ip); //inet_addr("127.0.0.1"); // INADDR_ANY;
	else
		_sin.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1"); // INADDR_ANY;
#endif

	int ret = ::bind(_ServerSocket, (sockaddr *)&_sin, sizeof(_sin));
	if (SOCKET_ERROR == ret)
	{
#ifdef _WIN32
		cout << "Bind " << ip << ":" << port << " error! Error code:" << WSAGetLastError() << endl;
		closesocket(_ServerSocket);
		WSACleanup();
#else
		cout << "Bind " << ip << ":" << port << " error!" << endl;
		close(_ServerSocket);
#endif
	}
	return ret;
}

int EasyTcpServer::Listen(int n)
{
	// the third listen
	int ret = listen(_ServerSocket, n);
	if (SOCKET_ERROR == ret)
	{
		cout << "Listen <socket=" << (int)_ServerSocket << "> error!" << endl;
#ifdef _WIN32
		closesocket(_ServerSocket);
		WSACleanup();
#else
		close(_ServerSocket);
#endif
	}
	else
	{
		cout << "Starup server..." << endl
			<< "Wait for client connect...port:5566 " << endl;
	}
	return ret;
}

SOCKET EasyTcpServer::Accept()
{
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
		cerr << "Error! accept Invalid socket. error code: " << WSAGetLastError() << endl;
#else
		cerr << "Error! accept Invalid socket." << endl;
#endif
	}
	else
	{
		g_clientList.push_back(_cSock);
		cout << inet_ntoa(_clientAddr.sin_addr) << "<" << (int)_cSock << ">join." << endl;
		broadcast(_cSock);
	}
	return _cSock;
}

bool EasyTcpServer::OnRun()
{
	// Select Model:
	// the frist Create and Init set.
	if (isRun())
	{
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
			CloseScok();
			return false;
		}

		// read branch:some client send data,server(server's socket[listen socket] had readly data) will to read data [read event]
		if (FD_ISSET(_ServerSocket, &readfds))
		{
			FD_CLR(_ServerSocket, &readfds);
			Accept();
		}
		// deal request [some had connective client send request(data) to server]
		// if server need to send data to client,jump to write branch -->
		// wirte brand:send data to some client,server's socket[listen socket] will write data [read event]
		// ѭ������һ���Ѿ��ڿɶ�����[read-set]�еĿͻ���socket

		for (int n = (int)g_clientList.size() - 1; n >= 0; n--) {
			if (FD_ISSET(g_clientList[n], &readfds)) {
				if (-1 == RecvData(g_clientList[n])) {
					auto g_clientList_iter = g_clientList.begin() + n;
					if (g_clientList_iter != g_clientList.end())
					{
						g_clientList.erase(g_clientList_iter);
					}
				}
			}
		}
		return true;
	}
	return false;
}

bool EasyTcpServer::isRun()
{
	return _ServerSocket != INVALID_SOCKET;
}

char szRecv[409600]{};
int EasyTcpServer::RecvData(SOCKET _cSock)
{
	// fifth receive-send and deal request
	int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		cout << "Client <" << (int)_cSock << ">was exit!" << endl;
		return -1; // client was exit/death.
	}
	//cout << "From Client<" << (int)_cSock << "> request: " << header->cmd
	//	<< " DataLength: " << header->dataLength << endl;
	recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
	OnNetMsg(_cSock, header);
	return 0;
}

void EasyTcpServer::OnNetMsg(SOCKET _cSock, DataHeader* header)
{
	switch (header->cmd)
	{
	case DPT_LOGIN:
	{
		Login* login = (Login*)header;
		// login sucess!
		if (!strcmp("User123", login->UserName) && !strcmp("password123", login->PassWord))
		{
			LoginResult ret{};
			ret.cmd = DPT_LOGINOK;
			ret.result = DPT_LOGINOK;
			sendToClient(_cSock, &ret);
		}
		else // login fail!
		{
			LoginResult ret{};
			ret.cmd = DPT_LOGINERROR;
			ret.result = DPT_LOGINERROR;
			sendToClient(_cSock, &ret);
		}
	}
	break;
	case DPT_LOGOUT:
	{
		Loginout* loginout = (Loginout*)header;
		// loginout sucess!
		if (!strcmp("User123", loginout->UserName))
		{
			LoginoutResult ret{};
			ret.result = DPT_LOGOUTOK;
			cout << loginout->UserName << " loginout." << endl;
			sendToClient(_cSock, &ret);
		}
		else // loginout fail!
		{
			LoginoutResult ret{};
			ret.cmd = DPT_LOGOUTERROR;
			ret.result = DPT_LOGOUTERROR;
			sendToClient(_cSock, &ret);
		}
	}
	break;
	case DPT_MSG:
	{
		Message* msgbuf = (Message*)header;
		Message facebackmsg;
		strcpy(facebackmsg.msg, msgbuf->msg);
		sendToClient(_cSock, &facebackmsg);
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
}

int EasyTcpServer::sendToClient(SOCKET _cSock, DataHeader *dataPack)
{
	if (isRun() && dataPack)
		return (int)send(_cSock, (const char*)dataPack, dataPack->dataLength, 0);
	else
		return SOCKET_ERROR;
}

void EasyTcpServer::broadcast(SOCKET _cSock) {
	NewUserJoinInfo broadcastInfo{};
	broadcastInfo.socketId = (int)_cSock;
	for (int n = (int)g_clientList.size() - 1; n >= 0; n--)
		sendToClient(g_clientList[n], &broadcastInfo);
}

SOCKET EasyTcpServer::getSock()
{
	return _ServerSocket;
}

void EasyTcpServer::CloseScok()
{
	if (_ServerSocket != INVALID_SOCKET)
	{
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
	}
}

#endif