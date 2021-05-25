#ifndef _CELL_IOCP_HPP_
#define _CELL_IOCP_HPP_
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

#include<mswsock.h>

#include<stdio.h>

enum IO_TYPE
{
	TYPE_ACCEPT = 10,
	TYPE_RECV,
	TYPE_SEND
};

#define IOCP_DATA_BUFF_SIZE 1024
struct IO_DATA_BASE
{
	//重叠体
	OVERLAPPED overlapped;
	//
	SOCKET sockfd;
	//数据缓冲区
	char buffer[IOCP_DATA_BUFF_SIZE];
	//实际缓冲区数据长度
	int length;
	//操作类型
	IO_TYPE iotype;
};

struct IOCP_EVENT
{
	IO_DATA_BASE* pIOData;
	SOCKET cSock = INVALID_SOCKET;
	DWORD bytesTrans = 0;
};

class CellIOCP
{
public:
	~CellIOCP()
	{
		destory();
	}

	bool Create()
	{
		// 创建完成端口IOCP
		_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (NULL == _completionPort)
		{
			printf("CreateIoCompletionPort fail with error %d\n", GetLastError());
			return false;
		}
		return true;
	}

	void destory()
	{
		if (_completionPort)
		{
			CloseHandle(_completionPort);
			_completionPort = NULL;
		}
	}

	bool reg(SOCKET sockfd)
	{
		HANDLE ret = CreateIoCompletionPort((HANDLE)sockfd, _completionPort, (ULONG_PTR)sockfd, 0);
		if (NULL == ret)
		{
			printf("CreateIoCompletionPort fail with error %d\n", GetLastError());
			return false;
		}
		return true;
	}

	//向IOCP投递接受连接任务
	void postAccept(IO_DATA_BASE* pIO_DATA)
	{
		if (!_lpfnAcceptEx)
			printf("error, postAccept() _lpfnAcceptEx is null\n");

		pIO_DATA->iotype = IO_TYPE::TYPE_ACCEPT;
		pIO_DATA->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (FALSE == _lpfnAcceptEx(
			_sockServer,
			pIO_DATA->sockfd,
			pIO_DATA->buffer,
			0,
			sizeof(sockaddr_in) + 16,
			sizeof(sockaddr_in) + 16,
			NULL,
			&pIO_DATA->overlapped
		))
		{
			int err = WSAGetLastError();
			if (ERROR_IO_PENDING != err)
			{
				printf("AcceptEx() fail errno:%d\n", err);
				return;
			}
		}
	}

	//向IOCP投递接收数据任务
	void postRecv(IO_DATA_BASE* pIO_DATA)
	{
		pIO_DATA->iotype = IO_TYPE::TYPE_RECV;
		WSABUF wsBuff = {};
		wsBuff.buf = pIO_DATA->buffer;
		wsBuff.len = IOCP_DATA_BUFF_SIZE;
		DWORD flags = 0;
		ZeroMemory(&pIO_DATA->overlapped, sizeof(OVERLAPPED));

		if (SOCKET_ERROR == WSARecv(
			pIO_DATA->sockfd,
			&wsBuff,
			1,
			NULL,
			&flags,
			&pIO_DATA->overlapped,
			NULL
		))
		{
			int err = WSAGetLastError();
			if (ERROR_IO_PENDING != err)
			{
				printf("WSARecv() fail. errno:%d\n", err);
				return;
			}
		}
	}

	//向IOCP投递发送数据任务
	void postSend(IO_DATA_BASE* pIO_DATA)
	{
		pIO_DATA->iotype = IO_TYPE::TYPE_SEND;
		WSABUF wsBuff = {};
		wsBuff.buf = pIO_DATA->buffer;
		wsBuff.len = pIO_DATA->length;
		DWORD flags = 0;
		ZeroMemory(&pIO_DATA->overlapped, sizeof(OVERLAPPED));

		if (SOCKET_ERROR == WSASend(
			pIO_DATA->sockfd,
			&wsBuff,
			1,
			NULL,
			flags,
			&pIO_DATA->overlapped,
			NULL
		))
		{
			int err = WSAGetLastError();
			if (ERROR_IO_PENDING != err)
			{
				printf("WSASend() fail. errno:%d\n", err);
				return;
			}
		}
	}

	int wait(IOCP_EVENT& ev,int timeout = 1)
	{
		ev.bytesTrans = 0;
		ev.pIOData = NULL;
		ev.cSock = INVALID_SOCKET;

		if (FALSE == GetQueuedCompletionStatus(
			_completionPort,
			&ev.bytesTrans,
			(PULONG_PTR)&ev.cSock,
			(LPOVERLAPPED*)&ev.pIOData,
			timeout
		))
		{
			int err = GetLastError();
			if (WAIT_TIMEOUT == err)
				return 0;

			if (ERROR_NETNAME_DELETED == err)
				return 1;

			printf("GetQueuedCompletionStatus:Overlapped I/O operation is in progress. errno:%d\n", GetLastError());
			return -1;
		}
		return 1;
	}

	bool loadAcceptEx(SOCKET ListenSocket)
	{
		if (INVALID_SOCKET != _sockServer)
		{
			printf("loadAcceptEx _sockServer != INVALID_SOCKET \n");
			return false;
		}
		if (_lpfnAcceptEx)
		{
			printf("loadAcceptEx _lpfnAcceptEx != NULL \n");
			return false;
		}
		_sockServer = ListenSocket;
		DWORD dwBytes = 0;
		GUID GuidAcceptEx = WSAID_ACCEPTEX;

		int iResult = WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx, sizeof(GuidAcceptEx),
			&_lpfnAcceptEx, sizeof(_lpfnAcceptEx),
			&dwBytes, NULL, NULL);
		if (iResult == SOCKET_ERROR)
		{
			printf("WSAIoctl failed with error: %u\n", WSAGetLastError());
			return false;
		}
		return true;
	}

private:
	LPFN_ACCEPTEX _lpfnAcceptEx = NULL;
	HANDLE _completionPort = NULL;
	SOCKET _sockServer = INVALID_SOCKET;
};
#endif //!_WIN32
#endif