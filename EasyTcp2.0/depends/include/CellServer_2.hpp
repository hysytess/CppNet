#ifndef _CELLSERVER2_HPP_
#define _CELLSERVER2_HPP_

#include "PublicLib.hpp"
#include "INETEVENT.hpp"
#include "ClientSocket.hpp"
#include "CELLSemaphore.hpp"
#include "CellFDSet.hpp"

#include <vector>
#include <map>

// 网络消息接受服务
class CellServer
{

public:
	CellServer(int id)
	{
		_id = id;
		_pNetEvent = nullptr;
		_taskServer._serverId = id;
	}
	~CellServer()
	{
		CellLog_Debug("CellServer%d.~CellServer exit.code:1", _id);
		Close();
		CellLog_Debug("CellServer%d.~CellServer exit.code:2", _id);

	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	void Close()
	{
		CellLog_Debug("CellServer%d closed.code:1", _id);
		_taskServer.Close();
		_cellThread.Close();
		CellLog_Debug("CellServer%d closed.code:2", _id);
	}

	void OnRun(CellThread* pThread)
	{
		while (pThread->isRun())
		{
			if (!_clientsBuff.empty())
			{
				std::lock_guard<std::mutex>lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					pClient->serverId = _id;
					if (_pNetEvent)
						_pNetEvent->OnNetJoin(pClient);
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			if (_clients.empty())
			{
				CellThread::Sleep(1);
				// 更新 心跳时间戳
				_old_time = CELLTime::getNowInMillisec();
				continue;
			}

			CheckTime();
			if (!DoSelect())
			{
				pThread->Exit();
				break;
			}
			DoMsg();
		}
		CellLog_Debug("CELLServer%d.OnRun exit", _id);
	}

	bool DoSelect()
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
			CellLog_Debug("CellServer%d.OnRun.Select error...exit.", _id);
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

	// 心跳检测
	void CheckTime()
	{
		auto nowTime = CELLTime::getNowInMillisec();
		auto dt = nowTime - _old_time;
		_old_time = nowTime;

		for (auto iter = _clients.begin(); iter != _clients.end();)
		{
			if (iter->second->checkHeart(dt))
			{
				if (_pNetEvent)
					_pNetEvent->OnLeave(iter->second);
				_clients_change = true;
				delete iter->second;
				auto iterOld = iter;
				iter++;
				_clients.erase(iterOld);
				continue;
			}
			//超时发送
			//iter->second->checkSend(dt);
			iter++;
		}
	}

	void onClientLeave(ClientSocket* pClient)
	{
		if (_pNetEvent)
			_pNetEvent->OnLeave(pClient);
		_clients_change = true;
		delete pClient;
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

	void DoMsg()
	{
		ClientSocket* pClient = nullptr;
		for (auto iter :_clients )
		{
			auto pClient = iter.second;
			while (pClient->hasMsg())
			{
				// 处理接收到完整的数据
				OnNetMsg(pClient, pClient->front_msg());
				// 移除在缓冲区队列头 已处理的数据
				pClient->pop_front_msg();
			}
		}
	}

	int RecvData(ClientSocket* pClient)
	{
		// 收数据
		int nLen = pClient->RecvData();
		// 判断接收情况
		if (nLen <= 0)
		{
			return -1;
		}
		// 触发接收事件 【pClient】 对象
		_pNetEvent->OnNetRecv(pClient);

		return 0;
	}

	virtual void OnNetMsg(ClientSocket* pClient, netmsg_DataHeader* header)
	{
		// 触发网络事件.
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

	void addClient(ClientSocket* pClient)
	{
		std::lock_guard<std::mutex>lock(_mutex);

		_clientsBuff.push_back(pClient);

	}

	void CellsrvStart()
	{

		_taskServer.Start();
		_cellThread.Start(
			// onCreate
			nullptr,
			// onRun
			[this](CellThread* pThread) {OnRun(pThread); },
			// onClose
			[this](CellThread* pThread) {Clearclients(); }
		);
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

	//void addSendTask(ClientSocket* pClient, netmsg_DataHeader* header)
	//{
	//	_taskServer.addTask([pClient, header]() {
	//		pClient->SendData(header);
	//		delete header;
	//	});
	//}

private:
	void Clearclients()
	{
		for (auto iter : _clients)
		{
			delete iter.second;
		}
		_clients.clear();

		for (auto iter : _clientsBuff)
		{
			delete iter;
		}
		_clientsBuff.clear();
	}

private:
	//Client sequen
	std::map<SOCKET, ClientSocket*>_clients;
	//Client sequen buff
	std::vector<ClientSocket*>_clientsBuff;
	std::mutex _mutex;
	INetEvent* _pNetEvent;
	CellTaskServer _taskServer;

	CellFDSet _fdRead_bak;
	CellFDSet _fdRead;
	CellFDSet _fdWrite;
	//fd_set fdExp;

	SOCKET _maxSock;
	time_t _old_time = CELLTime::getNowInMillisec();

	CellThread _cellThread;
	int _id = -1;
	// 有客户端加入 或者退出 集合 fd_set(fdRead) 改变
	bool _clients_change = true;
};

#endif