#ifndef _CLIENT_SOCKET_HPP_
#define _CLIENT_SOCKET_HPP_

#define CLIENT_HEART_DEAD_TIME 60000 //5000 ms
// 超时发送时间
#define CLIENT_SEND_BUFF_TIME 200 // ms

#include "CellMsgBuffer.hpp"

//客户端数据类型
class ClientSocket
{
public:
	int id = -1;
	int serverId = -1;
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET)
	{
		_sendBuff.setSize(SEND_BUFF_SZIE);
		_recvBuff.setSize(RECV_BUFF_SZIE);

		static int n = 1;
		id = n++;
		_sockfd = sockfd;

		resetDTHeart();
		resetDTSend();
	}
	~ClientSocket()
	{
		//printf("s=%d CellClient%d closed.code:1\n",serverId, id);
		if (INVALID_SOCKET != _sockfd)
		{
#ifdef _WIN32
			closesocket(_sockfd);
#else
			close(_sockfd);
#endif
			_sockfd = INVALID_SOCKET;
		}
	}

	SOCKET sockfd()
	{
		return _sockfd;
	}

	int RecvData()
	{
		return _recvBuff.read4socket(_sockfd);
	}

	bool hasMsg()
	{
		return _recvBuff.hasMsg();
	}

	netmsg_DataHeader* front_msg()
	{
		return (netmsg_DataHeader*)_recvBuff.data();
	}

	// 移除消息队列第一条
	void pop_front_msg()
	{
		if (hasMsg())
			_recvBuff.pop(front_msg()->dataLength);

	}

	int SendDataReal()
	{
		resetDTSend();
		return _sendBuff.write2socket(_sockfd);
	}

	//发送数据
	int SendData(netmsg_DataHeader* header)
	{
		if (_sendBuff.push((const char*)header,header->dataLength))
		{
			return header->dataLength;
		}
		
		return SOCKET_ERROR;
	}

	void resetDTHeart()
	{
		_dtHeart = 0;
	}

	void resetDTSend()
	{
		_dtSend = 0;
	}

	// 心跳检测
	bool checkHeart(time_t dt)
	{
		_dtHeart += dt;
		if (_dtHeart >= CLIENT_HEART_DEAD_TIME)
		{
			//printf("checkHeart dead:s=%d,time=%d\n", _sockfd, _dtHeart);
			return true;
		}
		return false;
	}
	// 超时发送
	bool checkSend(time_t dt)
	{
		_dtSend += dt;
		if (_dtSend >= CLIENT_SEND_BUFF_TIME)
		{
			// 立即发送在发送缓冲区中的数据
			// 重置超时计数 _dtSend
			//printf("Send timeout.\n");
			SendDataReal();
			resetDTSend();
			return true;
		}
		return false;
	}
private:
	// socket fd_set  file desc set
	SOCKET _sockfd;
	//接收消息缓冲区
	CellBuffer _recvBuff;
	//发送缓冲区
	CellBuffer _sendBuff;

	//心跳死亡计时
	time_t _dtHeart;
	// 发送超时时间
	time_t _dtSend;
	// 发送缓冲区溢满计数
	int _sendBuffFullCount = 0;
};

#endif // !_CLIENT_SOCKET_HPP_
