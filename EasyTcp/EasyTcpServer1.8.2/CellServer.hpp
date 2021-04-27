#ifndef _CELLSERVER_HPP_
#define _CELLSERVER_HPP_

#include "PublicLib.hpp"
#include "CELLTask.hpp"
#include "ClientSocket.hpp"

#include <vector>
#include <map>

// 网络消息发送任务
class CellSendMsg2ClientTask :public CellTask
{
	ClientSocket* _pClient;
	netmsg_DataHeader* _pHeader;
public:
	CellSendMsg2ClientTask(ClientSocket* pClient, netmsg_DataHeader* pHeader)
	{
		_pClient = pClient;
		_pHeader = pHeader;
	}

	// 执行任务
	void doTask()
	{
		_pClient->SendData(_pHeader);
		delete _pHeader;
	}
};

// 网络消息接受服务
class CellServer
{
private:
	SOCKET _sock;
	//Client sequen
	std::map<SOCKET, ClientSocket*>_clients;
	//Client sequen buff
	std::vector<ClientSocket*>_clientsBuff;
	std::mutex _mutex;
	std::thread* _pThread;
	INetEvent* _pNetEvent;
	CellTaskServer _taskServer;
public:
	std::atomic_int _recvCount;

	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
	}
	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
		delete _pThread;
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}

	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (auto iter : _clients)
			{
				closesocket(iter.second->sockfd());
				delete iter.second;
			}
			closesocket(_sock);
#else
			for (auto iter : _clients)
			{
				closesocket(iter.second->sockfd());
				delete iter.second;
			}
			close(_sock);
#endif
			_clients.clear();
		}
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	fd_set _fdRead_bak;
	// 有客户端加入 或者退出 集合 fd_set(fdRead) 改变
	bool _clients_change;
	SOCKET _maxSock;
	void OnRun()
	{
		_clients_change = true;
		while (isRun())
		{
			if (_clientsBuff.size() > 0)
			{
				std::lock_guard<std::mutex>lock(_mutex);
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			if (_clients.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
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

			timeval tv{ 0,0 };
			// 若要在CellServer中处理其他业务则用非阻塞模式
			int ret = (int)select(_maxSock + 1, &fdRead, nullptr, nullptr, &tv);
			if (ret < 0)
			{
				printf("Select done.\n");
				Close();
				return;
			}
			else if (ret == 0)
			{
				continue;
			}

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
						_clients.erase(iter->first);
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
						_clients_change = false;
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
		return;
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
		CellSendMsg2ClientTask* task = new CellSendMsg2ClientTask(pClient, header);
		_taskServer.addTask(task);
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
		_pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
		_taskServer.Start();
	}

	size_t getClientCount()
	{
		return _clients.size() + _clientsBuff.size();
	}
};

#endif