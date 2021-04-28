#ifndef _INETEVENT_HPP_
#define _INETEVENT_HPP_

#include "PublicLib.hpp"
#include "ClientSocket.hpp"

class CellServer;
// Delegation [consignor]
// {CellServer -infomation-> INetEvent -infomation-> EasyTcpServer(main work thread)}
class INetEvent
{
public:
	//client leave event.
	virtual void OnLeave(ClientSocket* pClient) = 0;
	// request event
	virtual void OnNetMsg(CellServer* pCellServer, ClientSocket* pClient, netmsg_DataHeader* header) = 0;
	// client join event
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	// receive data event
	virtual void OnNetRecv(ClientSocket* pClient) = 0;
};
#endif