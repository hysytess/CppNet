#include <iostream>
#include <algorithm>
#include <thread>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <cstdlib>

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

#include "MSGPROC.hpp"

std::vector<SOCKET> g_clientlist;

bool isExit = true;

int InitNetWork()
{
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	int err = WSAStartup(ver, &dat); // ����Windows ���绷��
	if (err != 0)
	{
		return -1;
	}
	if (LOBYTE(dat.wVersion) != 2 || HIBYTE(dat.wVersion) != 2)
	{
		WSACleanup();
		return -1;
	}
#endif
	return 1;
}

void ClearUp(SOCKET ServerSocket)
{
#ifdef _WIN32
	closesocket(ServerSocket);
	WSACleanup();
#else
	close(_ServerSocket);
#endif
}

SOCKET Create()
{
	// frist build a socket
// ipv4/6 stream tcp/udp
	SOCKET ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// the second binding port with ip adress;
	sockaddr_in _sin{};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567); // host to net unsigned short
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = INADDR_ANY; //inet_addr("127.0.0.1"); // INADDR_ANY;
#else
	_sin.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1"); // INADDR_ANY;
#endif

	if (INVALID_SOCKET == ServerSocket)
	{
		std::cout << "Create socket fail!" << std::endl;
		closesocket(ServerSocket);
		WSACleanup();
	}

	int ret = ::bind(ServerSocket, (sockaddr *)&_sin, sizeof(_sin));
	if (SOCKET_ERROR == ret)
	{
#ifdef _WIN32
		std::cout << "Bind error! Error code:" << WSAGetLastError() << std::endl;
		closesocket(ServerSocket);
		WSACleanup();
#else
		cout << "Bind error!" << endl;
		close(_ServerSocket);
#endif
	}

	if (SOCKET_ERROR == listen(ServerSocket, 5))
	{
		std::cout << "Listen error!" << std::endl;
#ifdef _WIN32
		closesocket(ServerSocket);
		WSACleanup();
#else
		close(_ServerSocket);
#endif
	}
	else
	{
		std::cout << "Starup server..." << std::endl
			<< "Wait for client connect...port:4567 " << std::endl;
	}
	return ServerSocket;
}

void MsgProc(SOCKET cSock)
{
	while (isExit) {
		char szRecv[1024]{};
		int nLen = recv(cSock, szRecv, sizeof(DataHeader), 0);
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			std::cout << "Client <" << (int)cSock << ">was exit!" << std::endl;
			break;
		}
		//std::cout << "From Client<" << (int)cSock << "> request: " << header->cmd
		//	<< " DataLength: " << header->dataLength << std::endl;

		switch (header->cmd)
		{
		case DPT_MSG:
		{
			recv(cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Message* msgbuf = (Message*)szRecv;
			Message facebackmsg;
			strcpy(facebackmsg.msg, msgbuf->msg);
			facebackmsg.receiver = msgbuf->receiver;
			facebackmsg.sender = cSock;
			send(facebackmsg.receiver, (char *)&facebackmsg, sizeof(Message), 0);
			std::cout << "From Client<" << (int)cSock << "> message: " << facebackmsg.msg << std::endl;
		}
		break;
		case DPT_EXIT:
		{
			if (!g_clientlist.empty())
			{
				std::cout << "From Client<" << (int)cSock << "> exit." << std::endl;
				auto it = std::find(g_clientlist.begin(), g_clientlist.end(), cSock);
				if (it != g_clientlist.end())
				{
					g_clientlist.erase(it);
				}
			}
		}
		break;
		default:
			break;
		}

	}
}

void Broadcast(SOCKET c)
{
	Message data;
	memset(data.msg,0,sizeof(data.msg));
	sprintf(data.msg, "New user<%d> join.", c);
	for (auto i : g_clientlist)
		send(i, (char*)&data,sizeof(Message),0);
}

void Accept(SOCKET ServerSocket)
{

	while (isExit)
	{
		sockaddr_in _clientAddr{};
		int nAdddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;

		cSock = accept(ServerSocket, (sockaddr*)&_clientAddr, &nAdddrLen);
		if (cSock != INVALID_SOCKET)
		{		
			//Broadcast(cSock);
			
			g_clientlist.push_back(cSock);
			Message data;
			memset(data.msg, 0, sizeof(data.msg));
			data.sender = ServerSocket;
			sprintf(data.msg, "Your ID %d.", cSock);
			send(cSock, (char*)&data, sizeof(Message), 0);

			std::thread t(&MsgProc, cSock);
			t.detach();
			std::cout << inet_ntoa(_clientAddr.sin_addr) << "<" << (int)cSock << ">join." << std::endl;
		}
	}
}

void GetCmd()
{
	std::string cmd;
	while (isExit)
	{
		std::cin >> cmd;
		if (!cmd.compare("exit"))
			isExit = false;
		cmd.clear();
	}
}

int main(int argc, char* args[])
{

	InitNetWork();
	SOCKET ServerSocket = Create();

	std::thread t2(&Accept,ServerSocket);
	t2.detach();

	GetCmd();

	ClearUp(ServerSocket);
	system("pause");
	return 0;
}