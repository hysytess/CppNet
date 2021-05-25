// 引入 Select 模型 支持一对多

#include <iostream>
#include <cstdlib>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define  _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#ifndef _WIN32 // WinSocket 动态库
#pragma comment(lib,"ws2_32.lib")
#endif

#include "MSGPROC.hpp"

using namespace std;

int processor(SOCKET _cSock);
int InitNetwork(int hibyte, int lobyte);
SOCKET CreateSocket();
int ConnectToHost(char *ip, int port, SOCKET _sock);

void ClearUp(SOCKET _cSock);

int main(int argc, char** argv) {
	if (InitNetwork(2, 2));
	else return 0;

	SOCKET _sock = CreateSocket();

	char ip[16] = "127.0.0.1";
	if (!ConnectToHost(ip, 4567, _sock));
	else return 0;

	// the third receive from server message
	while (true) {
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(_sock, &readfds);

		timeval tv{ 0,300 };
		int ret = select(_sock, &readfds, 0, 0, &tv);
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
		Message facebackmsg;
		strcpy(facebackmsg.msg, "hello!,i'm client.");
		send(_sock, (char*)&facebackmsg, sizeof(Message), 0);
		Sleep(1000);
		// Select space ... deal some business
	}

	ClearUp(_sock);
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
		cout << "Client <" << (int)_cSock << ">has disconnected witch server." << endl;
		return -1; // client was exit/death.
	}
	//cout << "From Client<" << (int)_cSock << "> request: " << header->cmd
	//	<< " DataLength: " << header->dataLength << endl;
	switch (header->cmd)
	{
	case DPT_LOGINOK:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginret = (LoginResult*)szRecv;
	}
	break;
	case DPT_LOGINERROR:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginResult* loginret = (LoginResult*)szRecv;
	}
	break;
	case DPT_LOGOUTOK:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginoutResult* logoutret = (LoginoutResult*)szRecv;
	}
	break;
	case DPT_LOGOUTERROR:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		LoginoutResult* logoutret = (LoginoutResult*)szRecv;
	}
	break;
	case DPT_MSG:
	{
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Message* msgbuf = (Message*)szRecv;
		Message facebackmsg;
		strcpy(facebackmsg.msg, msgbuf->msg);
		//cout << "From server's message: " << facebackmsg.msg << endl;
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
}

int InitNetwork(int hibyte, int lobyte) {
	WORD ver = MAKEWORD(hibyte, lobyte);
	WSADATA dat;
	WSAStartup(ver, &dat); // 启动Windows 网络环境
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
}

SOCKET CreateSocket() {
	// frist build a socket
		// ipv4/6 stream tcp/udp
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		cout << "Build socket fail! error code:" << WSAGetLastError() << endl;
		closesocket(_sock);
		WSACleanup();
		return 1;
	}
	return _sock;
}

int ConnectToHost(char *ip, int port, SOCKET _sock) {
	// the second connect to server;
	sockaddr_in _sin{};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
	int ret = connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)
	{
		cout << "Connect to server faild!error code:" << WSAGetLastError() << endl;
		closesocket(_sock);
		WSACleanup();
		return 1;
	}
	else
		cout << "<" << (int)_sock << "> Connected." << endl;
	return 0;
}

void ClearUp(SOCKET _cSock) {
	// foruth close socket
	closesocket(_cSock);
	WSACleanup();
}