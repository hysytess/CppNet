#ifndef _CellServer_IOCP_HPP_
#define _CellServer_IOCP_HPP_
#ifdef _WIN32

#include "CellServer_2.hpp"
#include "CellIOCP.hpp"

// 网络消息接受服务
class CellServer_IOCP:public CellServer
{

public:

	CellServer_IOCP()
	{
		_iocp.Create();
	}

	~CellServer_IOCP()
	{
		Close();
	}

	bool DoNetEvent()
	{
		//for (auto iter : _clients)
		//{
		//	if (iter.second->needWrite())
		//	{
		//		_ep.ctl(EPOLL_CTL_MOD,iter.second,EPOLLIN|EPOLLOUT);
		//	}
		//	else
		//	{
		//		_ep.ctl(EPOLL_CTL_MOD,iter.second,EPOLLIN);
		//	}
		//}

		int ret = DoIocpNetEvent();
		if (ret < 0)
		{
			perror("CellServer_Epoll.OnRun.Wait Eorr exit");
			return false;
		}
		else if (ret == 0)
			return true;

		return true; 
	}

	void rmClient(ClientSocket* pClient)
	{
		auto iter = _clients.find(pClient->sockfd());
		if (iter != _clients.end())
			_clients.erase(iter);
		
		onClientLeave(pClient);

	}

	void onClientJoin(ClientSocket* pClient)
	{
		_iocp.reg(pClient->sockfd(), pClient);
		auto pIoData = pClient->makeRecvIOData();
		if(pIoData)
			_iocp.postRecv(pIoData);
	}

	int DoIocpNetEvent()
	{
		int ret = _iocp.wait(_ioEvent, 1);
		if (ret < 0)
		{
			perror("CellServer_Epoll.OnRun.Wait Eorr exit");
			return ret;
		}
		else if (ret == 0)
			return ret;

		if (IO_TYPE::TYPE_RECV == _ioEvent.pIOData->iotype)
		{
			if (_ioEvent.bytesTrans <= 0)
			{
				CellLog_Debug("close socket<%d>. bytesTrans=%d\n", (int)_ioEvent.pIOData->sockfd, (int)_ioEvent.bytesTrans);
				CellNetWork::destorySocket(_ioEvent.pIOData->sockfd);
				_iocp.postAccept(_ioEvent.pIOData);
				return ret;
			}
			ClientSocket* pClient = (ClientSocket*)_ioEvent.data.ptr;
			if (pClient)
				pClient->recv4IOCP(_ioEvent.bytesTrans);
			
			CellLog_Debug("WSARecv() socket<%d>. bytesTrans=%d msgCount=%d \n", (int)_ioEvent.pIOData->sockfd, (int)_ioEvent.bytesTrans);
			
			//auto pIoData = pClient->makeRecvIOData();
			//if (pIoData)
			//	_iocp.postRecv(pClient->makeRecvIOData());
		}
		//else if (IO_TYPE::TYPE_SEND == _ioEvent.pIOData->iotype)
		//{
		//	if (_ioEvent.bytesTrans <= 0)
		//	{
		//		CellLog_Debug("close socket<%d>. bytesTrans=%d\n", (int)_ioEvent.pIOData->sockfd, (int)_ioEvent.bytesTrans);
		//		closesocket(_ioEvent.pIOData->sockfd);
		//		_iocp.postAccept(_ioEvent.pIOData);
		//		continue;
		//	}
		//	//printf("WSASend() socket<%d>. bytesTrans=%d msgCount=%d \n", (int)ioEvent.pIOData->sockfd, (int)ioEvent.bytesTrans, msgCount);
		//	_iocp.postRecv(_ioEvent.pIOData);
		//}
		else
		{
			CellLog_Debug("Invalid socket<%d>\n", (int)_ioEvent.data.fd);
		}
		return ret;
	}

private:
	CellIOCP _iocp;
	IOCP_EVENT _ioEvent = {};
};

#endif // !__linux__
#endif