#include <iostream>
#include <cstdlib>
#include <thread>
#include <stdio.h>
#include <stdlib.h>

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
#define SOCKET_ERROR   (SOCKET)(-1)
#endif

using namespace std;

bool g_isrun = true;

void Getcmd();
void SendHtml(SOCKET _cSock, const char *filePath);
void accept_Requeset(SOCKET _cSock);

int main(int argc, char *argv[]) {
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	int err = WSAStartup(ver, &dat);
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
	_sin.sin_port = htons(80); // host to net unsigned short
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1"); // INADDR_ANY;
#else
	_sin.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1"); // INADDR_ANY;
#endif

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
	if (SOCKET_ERROR == listen(_ServerSocket, 5))
	{
		cout << "Listen error!" << endl;
#ifdef _WIN32
		WSACleanup();
		closesocket(_ServerSocket);
#else
		close(_ServerSocket);
#endif
		return 1;
	}
	else
	{
		cout << "Starup server..." << endl
			<< "Wait for client connect..." << endl;
	}

	while (g_isrun)
	{
		sockaddr_in _clientAddr{};
		int nAdddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;

		_cSock = accept(_ServerSocket, (sockaddr*)&_clientAddr, &nAdddrLen);
		if (_cSock == INVALID_SOCKET)
			cout << "Error! accept Invalid socket." << endl;
		else
		{
			accept_Requeset(_cSock);
			cout << inet_ntoa(_clientAddr.sin_addr) << "<" << (int)_cSock << ">join." << endl;
		}
		closesocket(_cSock);
	}

	std::thread t1(Getcmd);
	t1.detach();

	WSACleanup();
	closesocket(_ServerSocket);
	system("pause");
	return 0;
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

void SendHtml(SOCKET _cSock, const char *filePath)
{
	FILE *fd = fopen(filePath, "rt");
	if (fd)// has open
	{
		char data1[1024];
		while (!feof(fd))
		{
			memset(data1, 0, 1024);
			fread(&data1, 1024, 1, fd);
			send(_cSock, data1, strlen(data1), 0);
		}
		fclose(fd);
		FILE *fd = fopen("./html/Page2/linger.mp3", "rb");
		if (!strcmp("./html/Page2/index.html", filePath))
		{
			cout << "圣诞" << endl;
			char data[1024];
			while (!feof(fd))
			{
				memset(data, 0, 1024);
				fread(&data, 1024, 1, fd);
				send(_cSock, data, strlen(data), 0);
			}
		}
		fclose(fd);
	}
	else // fail
	{
		// 404 html/404NotFound.html
		char data[1024];
		FILE* errorPage_fd = fopen("html/404NotFound.html", "r");
		if (errorPage_fd) {
			while (!feof(errorPage_fd))
			{
				memset(data, 0, 1024);
				fread(&data, 1024, 1, errorPage_fd);
				//cout << data << endl;
				send(_cSock, data, strlen(data), 0);
			}
		}

		fclose(errorPage_fd);
	}
}

void accept_Requeset(SOCKET _cSock)
{
	char recvBuf[1024]{ 0 };
	// get and analysis request head
	int nLen = recv(_cSock, recvBuf, sizeof(recvBuf), 0);
	if (nLen <= 0)
	{
		cout << "Error request.<socket=" << (int)_cSock << ">" << endl;
#ifdef _WIN32
		closesocket(_cSock);
#else
		close(_cSock);
#endif
		return;
	}
	cout << recvBuf << endl;

	//send html
	char filePath[128]{};
	//strcpy(filePath, rand() % 2 ? "./html/Page2/index.html" : "./html/Page2/index.html");
	strcpy(filePath, "html/404NotFound.html");

	SendHtml(_cSock, filePath);
}