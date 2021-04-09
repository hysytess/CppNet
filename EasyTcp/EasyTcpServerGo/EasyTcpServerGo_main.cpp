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

using namespace std;

int main(int argc, char** argv) {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat); // 启动Windows 网络环境
	// frist build a socket;
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
	char msgbuf[128]{};
	char _recvBuf[128]{};

	_cSock = accept(_ServerSocket, (sockaddr*)&_clientAddr, &nAdddrLen);
	if (_cSock == INVALID_SOCKET)
		cout << "Error! accept Invalid socket." << endl;
	else
		cout << inet_ntoa(_clientAddr.sin_addr) << "<" << (int)_cSock << ">join." << endl;

	while (true) {
		// fifth receive-send and deal request
		int nLen = recv(_cSock, _recvBuf, 128, 0);
		if (nLen <= 0)
		{
			cout << "Client <" << (int)_cSock << ">was exit!" << endl;
			break;
		}
		if (!strcmp("login#123#password.", _recvBuf))
		{
			memset(msgbuf, 0, sizeof(msgbuf));
			strcpy(msgbuf, "Login sucess!");
			cout << "Client <" << (int)_cSock << "> Login." << endl;
			send(_cSock, msgbuf, (int)strlen(msgbuf) + 1, 0);
		}
		else {
			memset(msgbuf, 0, sizeof(msgbuf));
			strcpy(msgbuf, "Login fail!");
			cout << "From client <" << (int)_cSock << ">: " << _recvBuf << endl;
			send(_cSock, msgbuf, (int)strlen(msgbuf) + 1, 0);
		}
	}

	// close socket
	closesocket(_ServerSocket);
	WSACleanup();
	system("pause");
	return 0;
}