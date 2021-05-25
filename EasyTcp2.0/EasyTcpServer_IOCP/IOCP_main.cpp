
#include "CellIOCP.hpp"


#define nClient 10

int main(int argc, char* args[])
{
	const char* ip = "127.0.0.1";
	unsigned short port = 4567;

	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	// 创建成功会默认设置 WSA_FLAG_OVERLAPPED 默认支持IOCP
	//SOCKET sockServer = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKET sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sockServer)
	{
		printf("Error, Create socket fail...\n");
	}
	else
	{
		printf("<socket=%d> was created...\n", (int)sockServer);
	}

	sockaddr_in _sin{};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
	if (ip)
	{
		_sin.sin_addr.s_addr = inet_addr(ip);
	}
	else
	{
		_sin.sin_addr.s_addr = INADDR_ANY;
	}

	int retbind = bind(sockServer, (sockaddr*)&_sin, sizeof(_sin));
	if (SOCKET_ERROR == retbind)
	{
		printf("Error, bind port<%d> fail...\n", port);
	}
	else
	{
		printf("Port<%d> bind sucess...\n", port);
	}

	int retlisten = listen(sockServer, 64);
	if (SOCKET_ERROR == retlisten)
	{
		printf("<socket=%d> listen error...\n", (int)sockServer);
	}
	else
	{
		printf("<socket=%d> listen, wait for client connect...\n", (int)sockServer);
	}

	//---------------------------IOCP--------------------------//
	// 创建完成端口IOCP
	CellIOCP iocp;
	iocp.Create();

	// 关联IOCP、socket //ULONG_PTR:兼容64、32位 DWORD只支持32位
	iocp.reg(sockServer);

	// 向IOCP投递接受连接任务
	iocp.loadAcceptEx(sockServer);

	IO_DATA_BASE ioData[nClient] = {};
	for (int n = 0; n < nClient; n++)
	{
		iocp.postAccept(&ioData[n]);
	}

	IOCP_EVENT ioEvent = {};
	int msgCount = 0;
	while (true)
	{
		int retwait = iocp.wait(ioEvent, 1);
		if (retwait < 0)
			break;
		if(0 == retwait)
			continue;

		if (IO_TYPE::TYPE_ACCEPT == ioEvent.pIOData->iotype)
		{
			printf("New client<socket=%d> join.\n", (int)ioEvent.pIOData->sockfd);
			if (!iocp.reg(ioEvent.pIOData->sockfd))
			{
				//printf("CreateIoCompletionPort fail with error %d\n", GetLastError());
				closesocket(ioEvent.pIOData->sockfd);
				iocp.postAccept(ioEvent.pIOData);
				continue;
			}
			iocp.postRecv(ioEvent.pIOData);
		}
		else if (IO_TYPE::TYPE_RECV == ioEvent.pIOData->iotype)
		{
			if (ioEvent.bytesTrans <= 0)
			{
				printf("close socket<%d>. bytesTrans=%d\n", (int)ioEvent.pIOData->sockfd, (int)ioEvent.bytesTrans);
				closesocket(ioEvent.pIOData->sockfd);
				iocp.postAccept(ioEvent.pIOData);
				continue;
			}
			//printf("WSARecv() socket<%d>. bytesTrans=%d msgCount=%d \n", (int)ioEvent.pIOData->sockfd, (int)ioEvent.bytesTrans, ++msgCount);
			ioEvent.pIOData->length = ioEvent.bytesTrans;
			iocp.postSend(ioEvent.pIOData);
		}
		else if (IO_TYPE::TYPE_SEND == ioEvent.pIOData->iotype)
		{
			if (ioEvent.bytesTrans <= 0)
			{
				printf("close socket<%d>. bytesTrans=%d\n", (int)ioEvent.pIOData->sockfd, (int)ioEvent.bytesTrans);
				closesocket(ioEvent.pIOData->sockfd);
				iocp.postAccept(ioEvent.pIOData);
				continue;
			}
			//printf("WSASend() socket<%d>. bytesTrans=%d msgCount=%d \n", (int)ioEvent.pIOData->sockfd, (int)ioEvent.bytesTrans, msgCount);
			iocp.postRecv(ioEvent.pIOData);
		}
		else
		{
			printf("Invalid socket<%d>\n", (int)ioEvent.pIOData->sockfd);
		}
	}

	closesocket(sockServer);

	for (int n = 0; n < nClient; n++)
	{
		closesocket(ioData[n].sockfd);
	}
	
	iocp.destory();
	WSACleanup();
	return 0;
}