// �����ƽ̨����
// ������߳��������̨

#include <iostream>
#include <cstdlib>
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
#define SOCKET_ERROR   (SOCKET)(-1)
#endif

#include "MSGPROC.hpp"

using namespace std;

bool g_isrun = true;

int processor(SOCKET _cSock);
int InitNetwork(int hibyte, int lobyte);
SOCKET CreateSocket();
int ConnectToHost(char *ip, int port, SOCKET _sock);

void ClearUp(SOCKET _cSock);
void GetCmd(SOCKET _cSock);

int main(int argc, char** argv) {
#ifdef _WIN32
	if (InitNetwork(2, 2));
	else return 0;
#endif
	SOCKET _sock = CreateSocket();

	char ip[16] = "127.0.0.1";
	if (!ConnectToHost(ip, 5566, _sock));
	else return 0;

	std::thread t1(GetCmd, _sock);
	t1.detach();

	// the third receive from server message
	while (g_isrun) {
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(_sock, &readfds);

		timeval tv{ 0,0 };
		// linux unix : maxsocket + 1
		int ret = select(_sock + 1, &readfds, 0, 0, &tv);
		if (ret < 0) {
			cout << "Select done." << endl;
			break;
		}
		if (FD_ISSET(_sock, &readfds))
		{
			FD_CLR(_sock, &readfds);
			if (-1 == processor(_sock)) {
				cout << "Select done." << endl;
				break;
			}
		}
		// Message facebackmsg{};
		// strcpy(facebackmsg.msg, "hello!,i'm client.");
		// send(_sock, (char*)&facebackmsg, sizeof(Message), 0);
		// Select space ... deal some business
	}

#ifdef _WIN32
	ClearUp(_sock);
	system("pause");
#else
	close(_sock);
#endif
	return 0;
}

int processor(SOCKET _cSock) {
	// fifth receive-send and deal request
	char szRecv[1024]{};
	int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0);
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		cout << "Client <" << (int)_cSock << ">has disconnected witch server." << endl;
		return -1; // client was exit/death.
	}
	//cout << "From Server<" << (int)_cSock << "> request: " << header->cmd
	//	<< " DataLength: " << header->dataLength << endl;
	switch (header->cmd)
	{
	case DPT_LOGINOK:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginret = (LoginResult*)szRecv;
		cout << "Login sucess!" << endl;
	}
	break;
	case DPT_LOGINERROR:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginret = (LoginResult*)szRecv;
		cout << "Login error!" << endl;
	}
	break;
	case DPT_LOGOUTOK:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginoutResult* logoutret = (LoginoutResult*)szRecv;
		cout << "Logout sucess!" << endl;
	}
	break;
	case DPT_LOGOUTERROR:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginoutResult* logoutret = (LoginoutResult*)szRecv;
		cout << "Login error!" << endl;
	}
	break;
	case DPT_MSG:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Message* msgbuf = (Message*)szRecv;
		Message facebackmsg;
		strcpy(facebackmsg.msg, msgbuf->msg);
		cout << "From server's message: " << facebackmsg.msg << endl;
	}
	break;
	case DPT_NEWUSER_JOIN:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		NewUserJoinInfo *nujibuf = (NewUserJoinInfo*)szRecv;
		cout << "New User<" << nujibuf->socketId << "> join." << endl;
	};
	break;
	default:
	{
		cout << "<socket= " << (int)_cSock << ">Error. cmd: " << header->cmd
			<< " dataLength: " << header->dataLength << endl;
	}
	break;
	}
	return 0;
}

#ifdef _WIN32
int InitNetwork(int hibyte, int lobyte) {
	WORD ver = MAKEWORD(hibyte, lobyte);
	WSADATA dat;
	WSAStartup(ver, &dat); // ����Windows ���绷��
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
}

void ClearUp(SOCKET _cSock) {
	// foruth close socket
	closesocket(_cSock);
	WSACleanup();
}
#endif

int ConnectToHost(char *ip, int port, SOCKET _sock) {
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
		cout << "Connect to server faild!error code:" << WSAGetLastError() << endl;
		closesocket(_sock);
		WSACleanup();
#else
		cout << "Connect to server faild!" << endl;
		close(_sock);
#endif
		return 1;
	}
	else
		cout << "<" << (int)_sock << "> Connected." << endl;
	return 0;
}

SOCKET CreateSocket() {
	// frist build a socket
		// ipv4/6 stream tcp/udp
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
#ifdef _WIN32
		cout << "Build socket fail! error code:" << WSAGetLastError() << endl;
		closesocket(_sock);
		WSACleanup();
#else
		cout << "Build socket fail!" << endl;
		close(_sock);
#endif
		return 1;
	}
	return _sock;
}

void GetCmd(SOCKET _cSock) {
	char cmdbuf[128]{};
	while (true) {
		memset(cmdbuf, 0, 128);
		cin >> cmdbuf;
		if (!strcmp(cmdbuf, "exit"))
		{
			g_isrun = false;
			break;
		}
		if (!strcmp(cmdbuf, "login")) {
			Login login{};
			strcpy(login.UserName, "User123");
			strcpy(login.PassWord, "password123");
			send(_cSock, (char*)&login, sizeof(Login), 0);
		}
		if (!strcmp(cmdbuf, "logout")) {
			Loginout logout{};
			strcpy(logout.UserName, "User123");
			send(_cSock, (char*)&logout, sizeof(Loginout), 0);
		}
		if (!strcmp(cmdbuf, "message")) {
			Message info;
			cin >> info.msg;
			send(_cSock, (char*)&info, sizeof(Message), 0);
		}
	}
}