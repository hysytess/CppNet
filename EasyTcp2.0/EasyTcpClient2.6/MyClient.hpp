#ifndef _MYCLIENT_HPP_
#define _MYCLIENT_HPP_

#include "../depends/include/EasyTcpClient_Select.hpp"
#include "../depends/include/EasyTcpClient_Epoll.hpp"
#include "../depends/include/CellConfig.hpp"

#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

std::atomic_int nConnect(0);
std::atomic_int sendcnt(0);
std::atomic_int readyCount(0);

const char* strIP = "127.0.0.1";
uint16_t nPort = 4567;
int nThread = 1;
int nClient = 1;
int nMsg = 1;
int nSendSleep = 1;
int nWorkSleep = 1;
int nSendBuffSize = SEND_BUFF_SZIE;
int nRecvBuffSize = RECV_BUFF_SZIE;

class MyClient : public EasyTcpClient_Select
{
public:
	MyClient()
	{
		_bCheckMsgID = CellConfig::Instance().hasKey("-checkMsgID");
	}
	
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			netmsg_LoginR* login = (netmsg_LoginR*)header;
			if (_bCheckMsgID)
			{
				if (login->msgID !=_nRecvMsgID)
				{
					CellLog_Error("socket<%d> msgID<%d> _nRecvMsgID<%d> %d", _pClient->sockfd(), login->msgID,_nRecvMsgID,login->msgID-_nRecvMsgID);
				}
				++_nRecvMsgID;
			}
			//printf("<socket=%d>收到服务器消息：CMD_LOGIN_RESULT,数据长度：%d\n", _pClient->sockfd(), header->dataLength);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			netmsg_LogoutR* logout = (netmsg_LogoutR*)header;
			//printf("<socket=%d>收到服务器消息：CMD_LOGOUT_RESULT,数据长度：%d\n", _pClient->sockfd(), header->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			netmsg_NewUserJoin* userJoin = (netmsg_NewUserJoin*)header;
			//printf("<socket=%d>收到服务器消息：CMD_NEW_USER_JOIN,数据长度：%d\n",_pClient->sockfd(), header->dataLength);
		}
		break;
		case CMD_ERROR:
		{
			netmsg_NewUserJoin* userJoin = (netmsg_NewUserJoin*)header;
			//printf("<socket=%d>收到服务器消息：CMD_ERROR,数据长度：%d\n", _pClient->sockfd(), header->dataLength);
		}
		break;
		default:
		{
			CellLog_Error("Error! <socket=%d>, dataLength: %d", (int)_pClient->sockfd(), header->dataLength);
		}
		}
	}

	int  SendTest(netmsg_Login* login)
	{
		int ret = 0;
		if (_nSendCount > 0)
		{
			login->msgID = _nSendMsgID;
			ret = SendData(login);
			if (SOCKET_ERROR != ret)
			{
				++_nSendMsgID;
				//发送剩余次数减少一次
				--_nSendCount;
			}
		}
		return ret;
	}

	bool checkSend(time_t dt)
	{
		_tRestTime += dt;
		//每过nSendSleep毫秒
		if (_tRestTime >= nSendSleep)
		{
			//重置计时
			_tRestTime -= nSendSleep;
			//重置发送计数
			_nSendCount = nMsg;
		}
		return _nSendCount > 0;
	}

private:
	int _nSendCount = 0;
	int _nRecvMsgID = 1;
	int _nSendMsgID = 1;
	time_t _tRestTime = 0;
	bool _bCheckMsgID = false;
};

#endif
