#ifndef _CELLSERVER_HPP_
#define _CELLSERVER_HPP_

#include "PublicLib.hpp"
#include "INETEVENT.hpp"
#include "ClientSocket.hpp"
#include "CELLSemaphore.hpp"

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
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				// 更新 心跳时间戳
				_old_time = CELLTime::getNowInMillisec();
				continue;
			}

			CheckTime();

			fd_set fdRead;
			fd_set fdWrite;
			//fd_set fdExp;

			if (_clients_change)
			{
				_clients_change = false;
				FD_ZERO(&fdRead);
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					if (_maxSock < iter.second->sockfd())
					{
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}

			bool bNeedWrite = false;

			//检测是否有可写客户端
			FD_ZERO(&fdWrite);
			for (auto iter : _clients )
			{
				if (iter.second->needWrite())
				{
					bNeedWrite = true;
					FD_SET(iter.second->sockfd(),&fdWrite);
				}
			}

			//memcpy(&fdWrite, &_fdRead_bak, sizeof(fd_set));
			//memcpy(&fdExp, &_fdRead_bak, sizeof(fd_set));
			 
			timeval tv{ 0,1 };
			int ret = 0;
			if (bNeedWrite)
			{
				ret = (int)select(_maxSock + 1, &fdRead, &fdWrite, nullptr, &tv);
			}
			else
			{
				ret = (int)select(_maxSock + 1, &fdRead, nullptr, nullptr, &tv);
			}
			if (ret < 0)
			{
				CellLog_Debug("CellServer%d.OnRun.Select error...exit.", _id);
				pThread->Exit();
				break;
			}
			else if (ret == 0)
			{
				continue;
			}

			ReadData(fdRead);
			WriteData(fdWrite);
			//WriteData(fdExp);
		}
		CellLog_Debug("CELLServer%d.OnRun exit", _id);
	};

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

	void WriteData(fd_set& fdWrite)
	{
#ifdef _WIN32

		for (int n = 0; n < fdWrite.fd_count; n++)
		{
			auto iter = _clients.find(fdWrite.fd_array[n]);
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
			if (iter->second->needWrite() && FD_ISSET(iter->second->sockfd(), &fdWrite))
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

	void ReadData(fd_set& fdRead)
	{
#ifdef _WIN32

		for (int n = 0; n < fdRead.fd_count; n++)
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
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
			if (FD_ISSET(iter->second->sockfd(), &fdRead))
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

		while (pClient->hasMsg())
		{
			// 处理接收到完整的数据
			OnNetMsg(pClient, pClient->front_msg());
			// 移除在缓冲区队列头 已处理的数据
			pClient->pop_front_msg();
		}
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

	fd_set _fdRead_bak;
	SOCKET _maxSock;
	time_t _old_time = CELLTime::getNowInMillisec();
	CellThread _cellThread;
	int _id = -1;
	// 有客户端加入 或者退出 集合 fd_set(fdRead) 改变
	bool _clients_change = true;
};

#endif