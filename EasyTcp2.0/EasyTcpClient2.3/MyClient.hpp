#ifndef _MYCLIENT_HPP_
#define _MYCLIENT_HPP_

#include "../depends/include/EasyTcpClient2_3.hpp"

class MyClient : public EasyTcpClient
{
public:
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			netmsg_LoginR* login = (netmsg_LoginR*)header;
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
private:
};

#endif
