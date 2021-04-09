// EasyTcpServer v1.0 引入结构化消息体

#include <iostream>
#include <cstdlib>

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

int main(int argc, char** argv) {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat); // 启动Windows 网络环境

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
		cout << "Bind error!" << endl;
	}
	else
		cout << "Bind success!" << endl;

	// the third listen
	if (SOCKET_ERROR == listen(_ServerSocket, 5)) // 监听套接字 最大监听队列
	{
		cout << "Listen error!" << endl;
	}
	else
		cout << "Listen success!" << endl;

	// fourth accept request
	sockaddr_in _clientAddr{};
	int nAdddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = INVALID_SOCKET;
	DataHeader header{};

	_cSock = accept(_ServerSocket, (sockaddr*)&_clientAddr, &nAdddrLen);
	if (_cSock == INVALID_SOCKET)
		cout << "Error! accept Invalid socket." << endl;
	else
		cout << inet_ntoa(_clientAddr.sin_addr) << "<" << (int)_cSock << ">join." << endl;

	while (true) {
		// fifth receive-send and deal request
		int nLen = recv(_cSock, (char *)&header, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			cout << "Client <" << (int)_cSock << ">was exit!" << endl;
			break;
		}
		cout << "From Client<" << (int)_cSock << "> request: " << header.cmd
			<< " DataLength: " << header.dataLength << endl;
		switch (header.cmd)
		{
		case DPT_LOGIN:
		{
			Login login{};
			recv(_cSock, (char *)&login + sizeof(DataHeader), sizeof(Login) - sizeof(DataHeader), 0);
			// login sucess!
			if (!strcmp("User123", login.UserName) && !strcmp("password123", login.PassWord))
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
			Loginout loginout{};
			recv(_cSock, (char *)&loginout + sizeof(DataHeader), sizeof(Loginout) - sizeof(DataHeader), 0);
			// loginout sucess!
			if (!strcmp("User123", loginout.UserName))
			{
				LoginoutResult ret{};
				ret.result = DPT_LOGOUTOK;
				send(_cSock, (char *)&ret, sizeof(LoginoutResult), 0);
				cout << loginout.UserName << " loginout." << endl;
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
			Message msgbuf{};
			recv(_cSock, (char *)&msgbuf + sizeof(DataHeader), sizeof(Message) - sizeof(DataHeader), 0);
			send(_cSock, (char *)&msgbuf, sizeof(Message), 0);
			cout << "From Client<" << (int)_cSock << "> message: " << msgbuf.msg << endl;
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

	// sixth close socket
	closesocket(_ServerSocket);
	WSACleanup();
	system("pause");
	return 0;
}