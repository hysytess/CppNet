#ifndef _MYCLIENT_HPP_
#define _MYCLIENT_HPP_

#include "../depends/include/EasyTcpClient2_3.hpp"
#include "../depends/include/CellMsgStream.hpp"

class MyClient : public EasyTcpClient
{
public:
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGOUT_RESULT:
		{
			CellReadStream byteStream(header);
			byteStream.ReadInt16();
			byteStream.getNetCmd();
			auto a1 = byteStream.ReadInt8();
			auto a2 = byteStream.ReadInt16();
			auto a3 = byteStream.ReadInt32();
			auto a4 = byteStream.ReadInt64();
			auto a5 = byteStream.ReadFloat();
			auto a6 = byteStream.ReadDouble();

			char str1[10]{};
			auto a7 = byteStream.ReadArray(str1, 8);
			int pos1[3]{};
			auto a9 = byteStream.ReadArray(pos1, 3);

			char str2[10]{};
			auto a8 = byteStream.ReadArray(str2, 10);
			int pos2[6]{};
			auto a10 = byteStream.ReadArray(pos2, 6);

			//printf("<socket=%d>收到服务器消息：CMD_LOGIN_RESULT,数据长度：%d\n", _pClient->sockfd(), header->dataLength);
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
			CellLog::Info("Error! <socket=%d>, dataLength: %d\n", (int)_pClient->sockfd(), header->dataLength);
		}
		}
	}
};

#endif
