#ifndef _EasyTcpClient_IOCP_hpp_
#define _EasyTcpClient_IOCP_hpp_
#if _WIN32

#ifndef CELL_USE_IOCP
	#define CELL_USE_IOCP
#endif

#include "EasyTcpClient2_4.hpp"
#include "CellIOCP.hpp"

class EasyTcpClient_IOCP:public EasyTcpClient
{
public:
	virtual void OnInitSocket() 
	{
		_iocp.Create();
		_iocp.reg(_pClient->sockfd(),_pClient);
	}

	void Close()
	{
		_iocp.destory();
		EasyTcpClient::Close();
	}
	bool OnRun(int microseconds = 0)
	{
		if (isRun())
		{
			if (_pClient->needWrite())
			{
				auto pIoData = _pClient->makeSendIOData();
				if (pIoData)
				{
					if (!_iocp.postSend(pIoData))
					{
						Close();
						return false;
					}
				}
				pIoData = _pClient->makeRecvIOData();
				if (pIoData)
				{
					if (!_iocp.postRecv(pIoData))
					{
						Close();
						return false;
					}
				}
			}
			else
			{
				auto pIoData = _pClient->makeRecvIOData();
				if (pIoData)
				{
					if (!_iocp.postRecv(pIoData))
					{
						Close();
						return false;
					}
				}
			}
			
			while (true)
			{
				int ret = DoIocpNetEvent(microseconds);
				if (ret < 0)
				{
					perror("CellServer_Epoll.OnRun.Wait Eorr exit");
					return false;
				}
				else if (ret == 0)
				{
					DoMsg();
					return true;
				}
			}
			return true;
		}
		return false;
	}
protected:
	int DoIocpNetEvent(int microseconds = 0)
	{
		int ret = _iocp.wait(_ioEvent, microseconds);
		if (ret < 0)
		{
			perror("EasyIOCPClient.OnRun.Wait Eorr exit");
			return ret;
		}
		else if (ret == 0)
			return ret;

		if (IO_TYPE::TYPE_RECV == _ioEvent.pIOData->iotype)
		{

			if (_ioEvent.bytesTrans <= 0)
			{
				CellLog_Debug("close socket<%d>. bytesTrans=%d\n", (int)_ioEvent.pIOData->sockfd, (int)_ioEvent.bytesTrans);
				Close();
				return -1;
			}
			ClientSocket* pClient = (ClientSocket*)_ioEvent.data.ptr;
			if (pClient)
				pClient->recv4IOCP(_ioEvent.bytesTrans);

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
				Close();
				return -1;
			}
			ClientSocket* pClient = (ClientSocket*)_ioEvent.data.ptr;
			if (pClient)
				pClient->send2IOCP(_ioEvent.bytesTrans);
			//printf("WSASend() socket<%d>. bytesTrans=%d msgCount=%d \n", (int)ioEvent.pIOData->sockfd, (int)ioEvent.bytesTrans, msgCount);
			//_iocp.postRecv(_ioEvent.pIOData);
		}
		else
		{
			CellLog_Warring("undefine io type\n");
		}
		return ret;
	}

protected:
	CellIOCP _iocp;
	IOCP_EVENT _ioEvent = {};
};

#endif //!_WIN32
#endif