#ifndef _EasyTc_pClient2_3_hpp_
#define _EasyTc_pClient2_3_hpp_

#include "PublicLib.hpp"
#include "CellNetWork.hpp"
#include "ClientSocket.hpp"
#include "CellLog.hpp"

class EasyTcpClient
{
public:
	EasyTcpClient()
	{
		_isConnected = false;
	}

	virtual ~EasyTcpClient()
	{
		Close();
	}

	void InitSocket(int sendSize = SEND_BUFF_SZIE,int recvSize = RECV_BUFF_SZIE)
	{
		CellNetWork::Init();
		if (_pClient)
		{
			CellLog::Info("<socket=%d>old connection was disconnected...\n", (int)_pClient->sockfd());
			Close();
		}
		SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			CellLog::Info("Error, create socket<%d> fail...\n", (int)_sock);
		}
		else
		{
			_pClient = new ClientSocket(_sock,sendSize,recvSize);
			//CellLog::Info("Create socket=<%d> sucess...\n", (int)_sock);
		}
	}

	int Connect(const char* ip, unsigned short port)
	{
		if (!_pClient)
		{
			InitSocket();
		}
		sockaddr_in _sin{};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		if (ip)
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif // _WIN32
		//CellLog::Info("<socket=%d> connect to host<%s:%d>...\n", (int)_sock, ip, port);
		int ret = connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			CellLog::Info("<socket=%d> error, connect to host<%s:%d> fail...\n", (int)_pClient->sockfd(), ip, port);
		}
		else
		{
			_isConnected = true;
			//CellLog::Info("<socket=%d> connect to host<%s:%d> sucess.\n", (int)_sock, ip, port);
		}
		return ret;
	}

	void Close()
	{
		if (_pClient)
		{
			delete _pClient;
			_pClient = nullptr;
		}
		_isConnected = false;
	}

	bool OnRun()
	{
		if (isRun())
		{
			SOCKET sock = _pClient->sockfd();
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(sock, &fdReads);

			fd_set fdWrite;
			FD_ZERO(&fdWrite);

			timeval tv = { 0,1 };
			int ret = 0;
			if (_pClient->needWrite())
			{
				FD_SET(sock, &fdWrite);
				ret = (int)select(sock + 1, &fdReads, &fdWrite, 0, &tv);
			}
			else
			{
				ret = (int)select(sock + 1, &fdReads, nullptr, nullptr, &tv);
			}
			
			if (ret < 0)
			{
				CellLog::Info("<socket=%d>select done. terminal code:1\n", (int)sock);
				Close();
				return false;
			}
			if (FD_ISSET(sock, &fdReads))
			{
				//FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(sock))
				{
					CellLog::Info("<socket=%d>select done. terminal code:2\n", (int)sock);
					Close();
					return false;
				}
			}

			if (FD_ISSET(sock, &fdWrite))
			{
				//FD_CLR(_sock, &fdReads);
				if (-1 == _pClient->SendDataReal()) // 
				{
					Close();
					return false;
				}
			}

			return true;
		}
		return false;
	}

	bool isRun()
	{
		return _pClient && _isConnected;
	}

	int RecvData(SOCKET csock)
	{
		if (isRun())
		{
			// 收数据
			int nLen = _pClient->RecvData();
			// 判断接收情况
			if (nLen > 0)
			{
				while (_pClient->hasMsg())
				{
					// 处理接收到完整的数据
					OnNetMsg(_pClient->front_msg());
					// 移除在缓冲区队列头 已处理的数据
					_pClient->pop_front_msg();
				}
			}
			return nLen;
		}
		return 0;
	}

	virtual void OnNetMsg(netmsg_DataHeader* header) = 0;
	
	int SendData(netmsg_DataHeader* header)
	{
		if (isRun())
			return _pClient->SendData(header);
		else return 0;
	}

	int SendData(const char* pData, int len)
	{
		if (isRun())
			return _pClient->SendData(pData, len);
		return 0;
	}

protected:
	ClientSocket* _pClient = nullptr;

private:
	bool _isConnected;
};

#endif