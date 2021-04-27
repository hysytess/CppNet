#ifndef _CLIENT_SOCKET_HPP_
#define _CLIENT_SOCKET_HPP_

class ClientSocket
{
private:
	SOCKET _sockfd;
	// 接收消息缓冲区
	char _szMsgBuf[RECV_BUFF_SZIE];
	// 数据尾部位置
	int _lastPos;
	// 发送消息缓冲区
	char _szSendBuf[SEND_BUFF_SZIE];
	int _lastSendPos;
public:
	ClientSocket(SOCKET sock = INVALID_SOCKET)
	{
		_sockfd = sock;
		memset(_szMsgBuf, 0, RECV_BUFF_SZIE);
		_lastPos = 0;

		memset(_szSendBuf, 0, SEND_BUFF_SZIE);
		_lastSendPos = 0;
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}
	char* msgBuf()
	{
		return _szMsgBuf;
	}
	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		_lastPos = pos;
	}

	int SendData(netmsg_DataHeader* header)
	{
		int ret = SOCKET_ERROR;
		// 发送的数据长度
		int nSendLen = header->dataLength;
		const char* pSendData = (const char*)header;
		while (true)
		{
			if (_lastSendPos + nSendLen >= SEND_BUFF_SZIE)
			{
				// 可拷贝的数据长度
				int nCopyLen = SEND_BUFF_SZIE - _lastSendPos;
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//计算剩余数据长度
				pSendData += nCopyLen;
				//剩余数据长度
				nSendLen -= nCopyLen;
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SZIE, 0);
				// 数据尾部清零
				_lastSendPos = 0;
				// 发送 错误/中断
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else
			{
				// 将要发送的数据 拷贝到发送缓冲区尾部
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				//计算数据尾部位置
				_lastSendPos += nSendLen;
				break;
			}
		}
		return ret;
	}
};

#endif