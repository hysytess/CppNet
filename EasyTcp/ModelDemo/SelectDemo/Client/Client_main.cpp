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

int main(int argc, char** argv) {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat); // 启动Windows 网络环境

	// frist build a socket
	// ipv4/6 stream tcp/udp
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
		cout << "Build socket<" << (int)_sock << "> fail!" << endl;
	else
	{	// the second connect to server;
		sockaddr_in _sin{};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(233);
		_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		int ret = connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
		if (ret == SOCKET_ERROR)
			cout << " Client<" << (int)_sock << "> connect to server faild!" << endl;
		else
			cout << " Client<" << (int)_sock << "> Connected." << endl;
	}
	// the third receive from server message
	char msgBuf[128]{};
	DataHeader hd{};
	while (true) {
		char szRecv[1024]{};
		memset(msgBuf, 0, sizeof(msgBuf));
		cin >> msgBuf;
		if (!strcmp("exit", msgBuf)) {
			hd.cmd = DPT_EXIT;
			hd.dataLength = sizeof(DataHeader);
			send(_sock, (char *)&hd, sizeof(DataHeader), 0);
			break;
		}
		if (!strcmp("login", msgBuf))
		{
			Login login{};
			cout << "User name:";
			cin >> login.UserName;
			cout << "Password:";
			cin >> login.PassWord;
			send(_sock, (char *)&login, sizeof(Login), 0);
			recv(_sock, szRecv, sizeof(LoginResult), 0);
			LoginResult* ret = (LoginResult*)szRecv;
			if (ret->result == DPT_LOGINOK)
				cout << "Login sucess." << endl;
			else
				cout << "Login fail!" << endl;
		}
		if (!strcmp("logout", msgBuf))
		{
			Loginout loginout{};
			cout << "User name:";
			cin >> loginout.UserName;
			send(_sock, (char *)&loginout, sizeof(Loginout), 0);
			recv(_sock, szRecv, sizeof(LoginoutResult), 0);
			LoginoutResult* outret = (LoginoutResult*)szRecv;
			if (outret->result == DPT_LOGOUTOK)
				cout << "Logout sucess." << endl;
			else
				cout << "Logout fail!" << endl;
		}
		if (!strcmp("message", msgBuf))
		{
			Message info1{};
			cin >> info1.msg;
			send(_sock, (char *)&info1, sizeof(Message), 0);
			recv(_sock, szRecv, sizeof(Message), 0);
			Message* info = (Message*)szRecv;
			cout << "From server message: " << info->msg << endl;
		}
	}

	// foruth close socket
	closesocket(_sock);
	WSACleanup();
	system("pause");
	return 0;
}