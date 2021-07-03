#include <iostream>
#include <algorithm>
#include <thread>
#include <string>
#include <set>
#include <map>
#include <cstdlib>

#include "MSGPROC.hpp"

#ifdef _WIN32
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

char msgBuf[128]{};
DataHeader hd{};
char szRecv[1024]{};

bool isRun = true;

void MsgProc(SOCKET sock)
{
	while (isRun)
	{
		memset(msgBuf, 0, sizeof(msgBuf));
		recv(sock, szRecv, sizeof(Message), 0);
		Message* info = (Message*)szRecv;
		std::cout << "From <" <<info->sender<< "> message: " << info->msg << std::endl;
	}
}

int main(int argc, char** argv) {
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat); // 启动Windows 网络环境

	// frist build a socket
	// ipv4/6 stream tcp/udp
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sock)
	{
		std::cout << "Build socket fail!" << std::endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	
	// the second connect to server;
	sockaddr_in _sin{};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
	if (ret == SOCKET_ERROR)
	{
		std::cout << "Connect to server faild! " << ret << std::endl;
		closesocket(sock);
		WSACleanup();
		return -1;
	}
	
	std::thread t(MsgProc, sock);
	t.detach();

	while (true) {
		memset(msgBuf, 0, sizeof(msgBuf));
		std::cin >> msgBuf;
		if (!strcmp("exit", msgBuf)) {
			hd.cmd = DPT_EXIT;
			hd.dataLength = sizeof(DataHeader);
			send(sock, (char *)&hd, sizeof(DataHeader), 0);
			break;
		}
		
		if (!strcmp("message", msgBuf))
		{
			Message info1{};
			std::cout << "To ";
			std::cin >> info1.receiver;
			std::cin >> info1.msg;
			send(sock, (char *)&info1, sizeof(Message), 0);
		}
	}

	// foruth close socket
	closesocket(sock);
	WSACleanup();
	system("pause");
	return 0;
}