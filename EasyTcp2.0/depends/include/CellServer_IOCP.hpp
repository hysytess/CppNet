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
		ClientSocket* pClient = nullptr;
		for (auto iter = _clients.begin(); iter != _clients.end();)
		{
			pClient = iter->second;
			if (pClient->needWrite())
			{
				auto pIoData = pClient->makeSendIOData();
				if (pIoData)
				{
					if (!_iocp.postSend(pIoData))
					{
						onClientLeave(pClient);
						if (iter != _clients.end())
							iter = _clients.erase(iter);
						continue;

					}
				}
			}
			else
			{
				auto pIoData = pClient->makeRecvIOData();
				if (pIoData)
				{
					if (!_iocp.postRecv(pIoData))
					{
						onClientLeave(pClient);
						if (iter != _clients.end())
							iter = _clients.erase(iter);
						continue;
					}
				}
			}
			iter++;
		}

		while (true)
		{
			int ret = DoIocpNetEvent();
			if (ret < 0)
			{
				perror("CellServer_Epoll.OnRun.Wait Eorr exit");
				return false;
			}
			else if (ret == 0)
				break;
		}
		return true; 
	}

	// 每次只处理一件网络事件
	// ret = -1 iocp error
	// ret =  0 nothing events
	// ret =  1 has net-events

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
				rmClient(_ioEvent);
				return ret;
			}
			ClientSocket* pClient = (ClientSocket*)_ioEvent.data.ptr;
			if (pClient) 
			{
				pClient->recv4IOCP(_ioEvent.bytesTrans);
				OnNetRecv(pClient);
			}
			
			CellLog_Debug("WSARecv() socket<%d>. bytesTrans=%d msgCount=%d \n", (int)_ioEvent.pIOData->sockfd, (int)_ioEvent.bytesTrans);
			
			//auto pIoData = pClient->makeRecvIOData();
			//if (pIoData)
			//	_iocp.postRecv(pClient->makeRecvIOData());
		}
		else if (IO_TYPE::TYPE_SEND == _ioEvent.pIOData->iotype)
		{
			if (_ioEvent.bytesTrans <= 0)
			{
				CellLog_Debug("close socket<%d>. bytesTrans=%d\n", (int)_ioEvent.pIOData->sockfd, (int)_ioEvent.bytesTrans);
				rmClient(_ioEvent);
				return ret;
			}
			ClientSocket* pClient = (ClientSocket*)_ioEvent.data.ptr;
			if (pClient)
				pClient->send2IOCP(_ioEvent.bytesTrans);
			//printf("WSASend() socket<%d>. bytesTrans=%d msgCount=%d \n", (int)ioEvent.pIOData->sockfd, (int)ioEvent.bytesTrans, msgCount);
			//_iocp.postRecv(_ioEvent.pIOData);
		}
		else
		{
			CellLog_Warring("undefine action.\n");
		}
		return ret;
	}

	void rmClient(ClientSocket* pClient)
	{
		auto iter = _clients.find(pClient->sockfd());
		if (iter != _clients.end())
			_clients.erase(iter);

		onClientLeave(pClient);
	}

	void rmClient(IOCP_EVENT ioEvent)
	{
		ClientSocket* pClient = (ClientSocket*)_ioEvent.data.ptr;
		if (pClient)
			rmClient(pClient);
	}

	void onClientJoin(ClientSocket* pClient)
	{
		_iocp.reg(pClient->sockfd(), pClient);
		auto pIoData = pClient->makeRecvIOData();
		if (pIoData)
			_iocp.postRecv(pIoData);
	}

private:
	CellIOCP _iocp;
	IOCP_EVENT _ioEvent = {};
};

#endif // !__linux__
#endif