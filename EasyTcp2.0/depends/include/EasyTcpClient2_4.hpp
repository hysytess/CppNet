#ifndef _EasyTc_pClient2_4_hpp_
#define _EasyTc_pClient2_4_hpp_

#include "PublicLib.hpp"
#include "CellNetWork.hpp"
#include "ClientSocket.hpp"
#include "CellLog.hpp"
#include "CellFDSet.hpp"

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

	SOCKET InitSocket(int sendSize = SEND_BUFF_SZIE,int recvSize = RECV_BUFF_SZIE)
	{
		CellNetWork::Init();
		if (_pClient)
		{
			CellLog_Debug("<socket=%d>old connection was disconnected...", (int)_pClient->sockfd());
			Close();
		}
		SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			CellLog_Debug("Error, create socket<%d> fail...", (int)_sock);
		}
		else
		{
			// 端口重用
			CellNetWork::make_reuseaddr(_sock);
			_pClient = new ClientSocket(_sock,sendSize,recvSize);
			//CellLog_Debug("Create socket=<%d> sucess...", (int)_sock);
			OnInitSocket();
		}
		return _sock;
	}

	int Connect(const char* ip, unsigned short port)
	{
		if (!_pClient)
		{
			if (INVALID_SOCKET == InitSocket())
			{
				return SOCKET_ERROR;
			}
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
		//CellLog_Debug("<socket=%d> connect to host<%s:%d>...", (int)_sock, ip, port);
		int ret = connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret)
		{
			CellLog_Debug("<socket=%d> error, connect to host<%s:%d> fail...", (int)_pClient->sockfd(), ip, port);
		}
		else
		{
			_isConnected = true;
			OnConnect();
			//CellLog_Debug("<socket=%d> connect to host<%s:%d> sucess.", (int)_sock, ip, port);
		}
		return ret;
	}

	virtual void Close()
	{
		if (_pClient)
		{
			delete _pClient;
			_pClient = nullptr;
		}
		_isConnected = false;
	}

	virtual bool OnRun(int microseconds = 1)  = 0;

	bool isRun()
	{
		return _pClient && _isConnected;
	}

	int RecvData()
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
	virtual void OnInitSocket() {};
	virtual void OnConnect() {};

protected:
	ClientSocket* _pClient = nullptr;

private:
	bool _isConnected;

	CellFDSet _fdReads;
	CellFDSet _fdWrite;
};

#endif