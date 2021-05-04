#ifndef _MYSERVER_HPP_
#define _MYSERVER_HPP_

#include "../depends/include/EasyTcpServer2_3.hpp"
#include "../depends/include/CellMsgStream.hpp"

//应用层
class MyServer :public EasyTcpServer
{
public:
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		EasyTcpServer::OnNetJoin(pClient);
		_clientMap.insert(std::pair<SOCKET, ClientSocket*>(pClient->sockfd(), pClient));
		//CellLog::Info("client<%d> join.\n", pClient->sockfd());
	}

	virtual void OnNetLeave(ClientSocket* pClient)
	{
		EasyTcpServer::OnLeave(pClient);
		if (_clientMap.count(pClient->sockfd()))
			_clientMap.erase(pClient->sockfd());
		//CellLog::Info("client<%d> leave.\n", (int)pClient->sockfd());
	}

	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, netmsg_DataHeader* header)
	{
		EasyTcpServer::OnNetMsg(pCellServer, pClient->sockfd(), header);
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			pClient->resetDTHeart();
			netmsg_Login *login = (netmsg_Login*)header;
			//CellLog::Info("收到客户端<socket=%d>请求：CMD_LOGIN，数据长度：%d, userName=%s passWord=%s", (int)cSock, login->dataLength, login->userName, login->PassWord);
			netmsg_LoginR ret;
			//实时发送 性能更好
			if (SOCKET_ERROR == pClient->SendData(&ret))
			{
				// 消息发送缓冲区满了,消息没发出去
				CellLog::Info("<Socket=%d> Send full buff.\n", pClient->sockfd());
			}
		}
		break;
		case CMD_LOGOUT:
		{
			pClient->resetDTHeart();

			CellReadStream byteStream(header);
			// 预读取消息长度数据 抛掉(占位符) 
			// 或由客户{MyClient:public EasyTcpClient,MyServer:public EasyTcpServer}读取
			byteStream.ReadInt16();
			byteStream.getNetCmd();
			auto a1 = byteStream.ReadInt8();
			auto a2 = byteStream.ReadInt16();
			auto a3 = byteStream.ReadInt32();
			auto a4 = byteStream.ReadInt64();
			auto a5 = byteStream.ReadFloat();
			auto a6 = byteStream.ReadDouble();

			char str1[10]{};
			int pos1[10]{};
			auto a7 = byteStream.ReadArray(str1, 10);
			auto a9 = byteStream.ReadArray(pos1, 10);

			char str2[10]{};
			int pos2[6]{};
			auto a8 = byteStream.ReadArray(str2, 10);
			auto a10 = byteStream.ReadArray(pos2, 6);

			///////////////////////SEND BACK///////////////
			CellWriteStream byteStream1;

			byteStream1.setNetCmd(CMD_LOGOUT_RESULT);

			byteStream1.WriteInt8(5);
			byteStream1.WriteInt16(6);
			byteStream1.WriteInt32(7);
			byteStream1.WriteInt64(8);

			byteStream1.WriteFloat(14.0);
			byteStream1.WriteDouble(15.0);

			
			char str[]{ "server." };
			byteStream1.WriteArray(str, strlen(str));
			int pos[2]{ 1,2 };
			byteStream1.WriteArray(pos, 2);

			char str0[5] = "zzz";
			byteStream1.WriteString(str0);
			int pos0[]{ 5,6,7 };
			byteStream1.WriteArray(pos0, 3);

			byteStream1.finsh();
			pClient->SendData(byteStream1.data(), byteStream1.length());
			//CellLog::Info("收到客户端<socket=%d>请求：CMD_LOGOUT，数据长度：%d, userName=%s", (int)cSock, login->dataLength, login->userName);
			//netmsg_LogoutR ret;
			//SendData(pClient, &ret);
		}
		break;
		case CMD_C2S_HEART:
		{
			pClient->resetDTHeart();
			netmsg_s2c_Heart ret;
			pClient->SendData(&ret);
			
		}
		break;
		default:
		{
			CellLog::Info("<socket=%d>Error infomation, dataLength: %d\n", (int)pClient->sockfd(), header->dataLength);
		}
		break;
		}
	}

	int SendData(ClientSocket* pClient, netmsg_DataHeader* header)
	{
		if (pClient->sockfd() && header)
		{
			return send(pClient->sockfd(), (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	void SendDataToAll(netmsg_DataHeader* header)
	{
		for (auto _clientMap_iter = _clientMap.begin(); _clientMap_iter != _clientMap.end(); _clientMap_iter++)
		{
			SendData(_clientMap_iter->second, header);
		}
	}
private:
	std::map<SOCKET, ClientSocket*>_clientMap;
};
#endif