#ifndef _CELLMSGBUFFER_HPP_
#define _CELLMSGBUFFER_HPP_

#include "PublicLib.hpp"

class CellBuffer
{
public:
	CellBuffer(int nSize = 8192)
	{ 
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}
	~CellBuffer()
	{ 
		if (!_pBuff)
		{
			delete[] _pBuff;
			_pBuff = nullptr;
		}
	}

	void setSize(int nSize = 8192)
	{
		_nSize = nSize;
		_pBuff = new char[_nSize];
	}

	int getSize()
	{
		return _nSize;
	}

	char * data()
	{
		return _pBuff;
	}

	bool push(const char* pData, int nLen)
	{
		// 缓冲区溢满拓展
		//if (_nLast+nLen > _nSize)
		//{
		//	char* buff = nullptr;
		//	int n = _nLast + nLen - _nSize;
		//	if (n<=1024)
		//		buff = new char[_nSize + 1024];
		//	if (n>1024)
		//		buff = new char[_nSize + 5120];
		//	memcpy(buff,_pBuff,_nLast);
		//	delete[] _pBuff;
		//	_pBuff = buff;
		//}

		if (_nLast + nLen <= _nSize)
		{
			//将要发送的数据 拷贝到发送缓冲区尾部
			memcpy(_pBuff + _nLast, pData, nLen); //bug
			//计算数据尾部位置
			_nLast += nLen; //bug

			if (_nLast == SEND_BUFF_SZIE)
			{
				_buffFullCount++;
			}
			return true;
		}
		else
		{
			_buffFullCount++;
		}
		return false;
	}

	void pop(int nLen)
	{
		int n = _nLast - nLen;
		if (n > 0)
		{
			memcpy(_pBuff, _pBuff + nLen, n);
		}
		_nLast = n;

		if (_buffFullCount > 0)
			--_buffFullCount;
	}

	int write2socket(SOCKET _sockfd)
	{
		int ret = 0;
		if (_nLast > 0 && INVALID_SOCKET != _sockfd)
		{
			ret = send(_sockfd, _pBuff, _nLast, 0);
			if (ret <= 0)
			{
				return SOCKET_ERROR;
			}
			if (ret == _nLast)
			{
				_nLast = 0;
			}
			else
			{
				_nLast -= ret;
				memcpy(_pBuff, _pBuff + ret, _nLast);
			}
			_buffFullCount = 0;
		}
		return ret;
	}

	int read4socket(SOCKET sockfd)
	{
		if (_nSize - _nLast>0)
		{
			char* szRecv = _pBuff + _nLast;
			int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);
			if (nLen <= 0)
			{
				return SOCKET_ERROR;
			}
			// 缓冲区数据尾部后移
			_nLast += nLen;
			return nLen;
		}
		return 0;
	}

	bool hasMsg()
	{
		if (_nLast >= sizeof(netmsg_DataHeader))
		{
			netmsg_DataHeader* header = (netmsg_DataHeader*)_pBuff;
			return _nLast >= header->dataLength;
		}
		return false;
	}

	bool needWrite()
	{
		return _nLast > 0;
	}
private:
	//第二缓冲区
	char* _pBuff = nullptr;
	//缓冲区的数据尾部位置
	int _nLast = 0;
	//缓冲区大小
	int _nSize = 0;
	// 缓冲区溢满计数
	int _buffFullCount = 0;
};

#endif
