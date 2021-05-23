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

	virtual ~CellServer()
	{
		CellLog_Debug("CellServer%d.~CellServer exit.code:1", _id);
		Close();
		CellLog_Debug("CellServer%d.~CellServer exit.code:2", _id);

	}


	void setID(int id)
	{
		_id = id;
		_taskServer._serverId = id;
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
					onClientJoin(pClient);
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
			if (!DoNetEvent())
			{
				pThread->Exit();
				break;
			}
			DoMsg();
		}
		CellLog_Debug("CELLServer%d.OnRun exit", _id);
	}

	// 基类
	virtual bool DoNetEvent() = 0;

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

	virtual void onClientJoin(ClientSocket* pClient){}

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

protected:
	//Client sequen
	std::map<SOCKET, ClientSocket*>_clients;
private:
	//Client sequen buff
	std::vector<ClientSocket*>_clientsBuff;
	std::mutex _mutex;
	INetEvent* _pNetEvent = nullptr;
	CellTaskServer _taskServer;

	time_t _old_time = CELLTime::getNowInMillisec();

	CellThread _cellThread;
protected:
	int _id = -1;
	// 有客户端加入 或者退出 集合 fd_set(fdRead) 改变
	bool _clients_change = true;
};

#endif