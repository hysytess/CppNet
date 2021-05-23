#ifndef _CellServer_Select_Select_HPP_
#define _CellServer_Select_Select_HPP_

#include "CellServer_2.hpp"
#include "CellFDSet.hpp"

// 网络消息接受服务
class CellServer_Select:public CellServer
{

public:
	~CellServer_Select()
	{
		Close();
	}

	bool DoNetEvent()
	{

		if (_clients_change)
		{
			_clients_change = false;
			_fdRead.zero();
			_maxSock = _clients.begin()->second->sockfd();
			for (auto iter : _clients)
			{
				_fdRead.add(iter.second->sockfd());
				if (_maxSock < iter.second->sockfd())
				{
					_maxSock = iter.second->sockfd();
				}
			}
			
			//memcpy(_fdRead_bak.fdset(), _fdRead.fdset(), sizeof(fd_set));
			_fdRead_bak.copy(_fdRead);
		}
		else
		{
			//memcpy(_fdRead.fdset(), _fdRead_bak.fdset(), sizeof(fd_set));
			_fdRead.copy(_fdRead_bak);
		}

		bool bNeedWrite = false;

		//检测是否有可写客户端
		_fdWrite.zero();
		for (auto iter : _clients)
		{
			if (iter.second->needWrite())
			{
				bNeedWrite = true;
				_fdWrite.add(iter.second->sockfd());
			}
		}

		//memcpy(&fdWrite, &_fdRead_bak, sizeof(fd_set));
		//memcpy(&fdExp, &_fdRead_bak, sizeof(fd_set));

		timeval tv{ 0,1 };
		int ret = 0;
		if (bNeedWrite)
		{
			ret = (int)select(_maxSock + 1, _fdRead.fdset(), _fdWrite.fdset(), nullptr, &tv);
		}
		else
		{
			ret = (int)select(_maxSock + 1, _fdRead.fdset(), nullptr, nullptr, &tv);
		}
		if (ret < 0)
		{
			if (errno == EINTR)
				return true;

			CellLog_Debug("CellServer_Select%d.OnRun.Select error...exit.", _id);
			return false;
		}
		else if (ret == 0)
		{
			return true;
		}

		ReadData();
		WriteData();
		//WriteData(fdExp);
		return true; //
	}

	void WriteData()
	{
#ifdef _WIN32
		auto pfdset = _fdWrite.fdset();
		for (int n = 0; n < pfdset->fd_count; n++)
		{
			auto iter = _clients.find(pfdset->fd_array[n]);
			if (iter != _clients.end())
			{
				if (SOCKET_ERROR == iter->second->SendDataReal())
				{
					onClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
		}
		
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (iter->second->needWrite() && _fdWrite.has(iter->second->sockfd()))
			{
				if (SOCKET_ERROR == iter->second->SendDataReal())
				{
					onClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}

	void ReadData()
	{
#ifdef _WIN32
		auto pfdset = _fdRead.fdset();
		for (int n = 0; n < pfdset->fd_count; n++)
		{
			auto iter = _clients.find(pfdset->fd_array[n]);
			if (iter != _clients.end())
			{
				if (SOCKET_ERROR == RecvData(iter->second))
				{
					onClientLeave(iter->second);
					_clients.erase(iter);
				}
			}
		}
		
#else
		for (auto iter = _clients.begin(); iter != _clients.end(); )
		{
			if (_fdRead.has(iter->second->sockfd()))
			{
				if (SOCKET_ERROR == RecvData(iter->second))
				{
					onClientLeave(iter->second);
					auto iterOld = iter;
					iter++;
					_clients.erase(iterOld);
					continue;
				}
			}
			iter++;
		}
#endif
	}



private:

	CellFDSet _fdRead_bak;
	CellFDSet _fdRead;
	CellFDSet _fdWrite;
	//CellFDSet fdExp;

	SOCKET _maxSock;
};

#endif