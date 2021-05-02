#include "header.hpp"

#include <string>

extern "C"
{

////////////////////////////Test////////////////////////////
	EXPORT_DLL int Add(int a, int b)
	{
		return a + b;
	}

	EXPORT_DLL int Sub(int a, int b)
	{
		return a - b;
	}

////////////////////////////Client////////////////////////////
	//C# 回调
	// 这样写 把EXPORT_DLL 放在返回值之后有坑!!!
	//void* EXPORT_DLL CellClient_Create(void* csObj, OnNetMsgCallBack cbfun)
	
	EXPORT_DLL void* CellClient_Create(void* csObj, OnNetMsgCallBack cbfun)
	{
		NativeTCPClient* pClient = new NativeTCPClient();
		pClient->setCallBack(csObj, cbfun);
		return pClient;
	}

	EXPORT_DLL bool CellClient_Connect(NativeTCPClient* pClient,const char* ip,short port)
	{
		if (pClient && ip)
			return SOCKET_ERROR != pClient->Connect(ip, port);
		else false;
	}

	EXPORT_DLL bool CellClient_OnRun(NativeTCPClient* pClient)
	{
		if (pClient)
			return pClient->OnRun();
		else return false;
		
	}

	EXPORT_DLL void CellClient_Close(NativeTCPClient* pClient)
	{
		if (pClient)
		{
			pClient->Close();
			delete pClient;
		}
	}

	EXPORT_DLL int CellClient_SendData(NativeTCPClient* pClient, const char* data, int len)
	{
		if (pClient)
		{
			return pClient->SendData(data, len);
		}
		return 0;
	}
}


