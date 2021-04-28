#ifndef _CELLSERVER_HPP_
#define _CELLSERVER_HPP_

#include "PublicLib.hpp"
#include "CELLTask.hpp"
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
		printf("CellServer%d.~CellServer exit.code:1\n", _id);
		Close();
		printf("CellServer%d.~CellServer exit.code:2\n", _id);

	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	void Close()
	{
		printf("CellServer%d closed.code:1\n", _id);
		if (_isRun)
		{
			_taskServer.Close();
			_isRun = false;
			_sem.wait();
		}
		printf("CellServer%d closed.code:2\n", _id);	
	}

	void OnRun()
	{
		_clients_change = true;
		while (_isRun)
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

			fd_set fdRead;

			FD_ZERO(&fdRead);

			if (_clients_change)
			{
				_clients_change = false;
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					FD_SET(iter.second->sockfd(), &fdRead);
					_maxSock < iter.second->sockfd() ? _maxSock = iter.second->sockfd() : _maxSock;
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}

			timeval tv{ 0,1 };
			// 若要在CellServer中处理其他业务则用非阻塞模式
			int ret = (int)select(_maxSock + 1, &fdRead, nullptr, nullptr, &tv);
			if (ret < 0)
			{
				printf("Select done.\n");
				Close();
				return;
			}
			//else if (ret == 0)
			//{
			//	continue;
			//}

			ReadData(fdRead);
			CheckTime();
		}
		Clearclients();
		_sem.wakeup();
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
				auto iterOld = iter++;
				_clients.erase(iterOld);
				continue;
			}
			//超时发送
			iter->second->checkSend(dt);
			iter++;
		}
	}

	void ReadData(fd_set& fdRead)
	{
#ifdef _WIN32

		for (int n = 0; n < fdRead.fd_count; n++)
		{
			auto iter = _clients.find(fdRead.fd_array[n]);
			if (iter != _clients.end())
			{
				if (-1 == RecvData(iter->second))
				{
					if (_pNetEvent)
						_pNetEvent->OnLeave(iter->second);
					_clients_change = true;
					delete iter->second;
					_clients.erase(iter);
				}
			}
			else
			{
				std::cerr << "Oh,shit! what's error?" << std::endl;
			}
		}

#else
		std::vector<ClientSocket*>temp;
		for (auto iter : _clients)
		{
			if (FD_ISSET(iter.second->sockfd(), &fdRead))
			{
				if (-1 == RecvData(iter.second))
				{
					if (_pNetEvent)
						_pNetEvent->OnLeave(iter.second);
					_clients_change = true;
					close(iter->first);
					temp.push_back(iter.second);
				}
			}
		}
		for (auto pClient : temp)
		{
			_clients.erase(pClient->sockfd());
			delete pClient;
		}
#endif
	}

	int RecvData(ClientSocket* pClient)
	{
		char* szRecv = pClient->msgBuf() + pClient->getLastPos();
		int nLen = (int)recv(pClient->sockfd(), szRecv, (RECV_BUFF_SZIE)-pClient->getLastPos(), 0);
		// 触发事件 【pClient】 对象
		_pNetEvent->OnNetRecv(pClient);
		if (nLen <= 0)
		{
			return -1;
		}
		// 只要有数据过来 就认为 客户端还在.
		//pClient->resetDtHeart();
		//memcpy(pClient->msgBuf() + pClient->getLastPos(), szRecv, nLen);

		pClient->setLastPos(pClient->getLastPos() + nLen);

		while (pClient->getLastPos() >= sizeof(netmsg_DataHeader))
		{
			netmsg_DataHeader* header = (netmsg_DataHeader*)pClient->msgBuf();

			if (pClient->getLastPos() >= header->dataLength)
			{
				int nSize = pClient->getLastPos() - header->dataLength;
				OnNetMsg(pClient, header);
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				pClient->setLastPos(nSize);
			}
			else
			{
				break;
			}
		}
		return 0;
	}

	void addSendTask(ClientSocket* pClient, netmsg_DataHeader* header)
	{
		_taskServer.addTask([pClient, header]() {
			pClient->SendData(header);
			delete header;
		});
	}

	virtual void OnNetMsg(ClientSocket* pClient, netmsg_DataHeader* header)
	{
		// 触发网络事件.
		_pNetEvent->OnNetMsg(this, pClient, header);
	}

	void addClient(ClientSocket* pClient)
	{
		std::lock_guard<std::mutex>lock(_mutex);
		//_mutex.lock();
		_clientsBuff.push_back(pClient);
		//_mutex.unlock();
	}

	void CellsrvStart()
	{
		if (!_isRun)
		{
			_isRun = true;
			std::thread t(std::mem_fun(&CellServer::OnRun), this);
			t.detach();
			_taskServer.Start();
		}
		
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}

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
		CELLSemaphore _sem;
		int _id = 0;
		// 有客户端加入 或者退出 集合 fd_set(fdRead) 改变
		bool _clients_change = true;
		bool _isRun = false;
		
};

#endif