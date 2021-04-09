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

using namespace std;

int main(int argc, char** argv) {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat); // 启动Windows 网络环境
	// frist build a socket;
	// ipv4/6 stream tcp/udp
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		cout << "Build socket fail!" << endl;
	}
	else
		cout << "Building socket success." << endl;
	// the second connect to server;
	sockaddr_in _sin{};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(233);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)
	{
		cout << "Connect to server faild!" << endl;
	}
	else
		cout << "Connect to server sucess." << endl;
	// the third receive from server message
	char recvBuf[256]{};
	char msgbuf[128]{};
	while (true) {
		memset(recvBuf, 0, sizeof(recvBuf));
		memset(msgbuf, 0, sizeof(msgbuf));

		cin >> msgbuf;
		if (!strcmp("exit", msgbuf)) break;
		send(_sock, msgbuf, (int)strlen(msgbuf) + 1, 0);

		if (recv(_sock, recvBuf, 256, 0) > 0)
			cout << "From Server message: " << recvBuf << endl;
	}
	// foruth close socket
	closesocket(_sock);
	WSACleanup();
	system("pause");
	return 0;
}