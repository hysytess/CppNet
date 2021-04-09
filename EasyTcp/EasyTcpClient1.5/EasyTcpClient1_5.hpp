#ifndef CLIENT1_5_HPP
#define CLIENT1_5_HPP
#include "MSGPROC.hpp"
#include <iostream>
#include <cstdlib>
#include <thread>

#ifdef _WIN32

#define  _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGSs
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

class	EasyTcpClient
{
public:
	EasyTcpClient();
	virtual ~EasyTcpClient();

#ifdef _WIN32
	int InitNetwork(int hibyte, int lobyte);
#endif
	void CreateSocket();
	int ConnectToHost(char *ip, int port, SOCKET _sock);
	bool OnRun();
	bool isRun();
	int RecvData(SOCKET _sock);
	void OnNetMsg(DataHeader* header);
	int sendToHost(DataHeader *dataPack);
	SOCKET getSock();
	void CloseScok();

protected:
private:
	SOCKET _sock;
};

EasyTcpClient::EasyTcpClient()
{
#ifdef _WIN32

	//!0 InitNetwork fail.
	if (!InitNetwork(2, 2))
	{
		cerr << "Init network fail!\n";
		exit(1);
	}
#endif
	_sock = INVALID_SOCKET;
}

EasyTcpClient::~EasyTcpClient()
{
	CloseScok();
}

#ifdef _WIN32
// startup return 1 else 0
int EasyTcpClient::InitNetwork(int hibyte, int lobyte)
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

void EasyTcpClient::CreateSocket()
{
	// frist build a socket
	// ipv4/6 stream tcp/udp
	if (_sock != INVALID_SOCKET)
	{
		CloseScok();
		cerr << "<socket= " << (int)getSock() << ">disconnect old connection.\n";
	}

	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
#ifdef _WIN32
		cerr << "Build socket fail! error code:" << WSAGetLastError() << endl;
		closesocket(_sock);
		WSACleanup();
#else
		cerr << "Build socket fail!" << endl;
		close(_sock);
#endif
		return;
	}
}

// connected return 1 else 0
int EasyTcpClient::ConnectToHost(char *ip, int port, SOCKET _sock)
{
	if (_sock == INVALID_SOCKET)
	{
#ifdef _WIN32
		//!0 InitNetwork fail.
		if (!InitNetwork(2, 2))
		{
			cerr << "Init network fail!\n";
			exit(1);
		}
#endif
		CreateSocket();
	}

	// the second connect to server;
	sockaddr_in _sin{};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);

#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif

	int ret = connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)
	{
#ifdef _WIN32
		cerr << "Connect to server faild!error code:" << WSAGetLastError() << endl;
		closesocket(_sock);
		WSACleanup();
#else
		cerr << "Connect to server faild!" << endl;
		close(_sock);
#endif
		return 0;
	}
	else
		cout << "<" << (int)_sock << "> Host:" << ip << " Connected." << endl;
	return 1;
}

bool EasyTcpClient::OnRun()
{
	// the third receive from server message
	if (isRun())
	{
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(_sock, &readfds);

		timeval tv{ 0,0 };
		// linux unix : maxsocket + 1
		int ret = (int)select(_sock + 1, &readfds, 0, 0, &tv);
		if (ret < 0) {
			cout << "Select done." << endl;
			return false;
		}
		if (FD_ISSET(_sock, &readfds))
		{
			FD_CLR(_sock, &readfds);
			if (-1 == RecvData(_sock)) {
				cout << "Select done." << endl;
				return false;
			}
		}
		return true;
	}
	return false;
}

bool EasyTcpClient::isRun()
{
	return _sock != INVALID_SOCKET;
}

// recv and analysis divide netdata
int EasyTcpClient::RecvData(SOCKET _sock)
{
	// fifth receive-send and deal request
	char szRecv[4096]{};
	int nLen = (int)recv(_sock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		cout << "Client <" << (int)_sock << ">has disconnected witch server." << endl;
		return -1; // client was exit/death.
	}
	//cout << "From Server<" << (int)_sock << "> request: " << header->cmd
	//	<< " DataLength: " << header->dataLength << endl;

	recv(_sock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
	OnNetMsg(header);
	return 0;
}

void EasyTcpClient::OnNetMsg(DataHeader* header)
{
	switch (header->cmd)
	{
	case DPT_LOGINOK:
	{
		LoginResult* loginret = (LoginResult*)header;
		cout << "Login sucess!" << endl;
	}
	break;
	case DPT_LOGINERROR:
	{
		LoginResult* loginret = (LoginResult*)header;
		cout << "Login error!" << endl;
	}
	break;
	case DPT_LOGOUTOK:
	{
		LoginoutResult* logoutret = (LoginoutResult*)header;
		cout << "Logout sucess!" << endl;
	}
	break;
	case DPT_LOGOUTERROR:
	{
		LoginoutResult* logoutret = (LoginoutResult*)header;
		cout << "Login error!" << endl;
	}
	break;
	case DPT_MSG:
	{
		Message* msgbuf = (Message*)header;
		Message facebackmsg;
		strcpy(facebackmsg.msg, msgbuf->msg);
		if (!strcmp("hello!,i'm client.", facebackmsg.msg));
		else
			cout << "From server's message: " << facebackmsg.msg << endl;
	}
	break;
	case DPT_NEWUSER_JOIN:
	{
		NewUserJoinInfo *nujibuf = (NewUserJoinInfo*)header;
		cout << "New User<" << nujibuf->socketId << "> join." << endl;
	};
	break;
	default:
	{
		cout << "<socket= " << (int)_sock << ">Error. cmd: " << header->cmd
			<< " dataLength: " << header->dataLength << endl;
	}
	break;
	}
}

int EasyTcpClient::sendToHost(DataHeader *dataPack)
{
	if (isRun() && dataPack)
		return send(_sock, (char*)dataPack, dataPack->dataLength, 0);
	else
		return SOCKET_ERROR;
}

SOCKET EasyTcpClient::getSock()
{
	return _sock;
}

void EasyTcpClient::CloseScok()
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
}

#endif // MSGPROC_HPP