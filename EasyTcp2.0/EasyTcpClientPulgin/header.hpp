#ifndef _PLUGIN_HEADER_DLL_H_
#define _PLUGIN_HEADER_DLL_H_

#include<string>
#include"../depends/include/EasyTcpClient2_3.hpp"

#ifdef _WIN32
#define EXPORT_DLL _declspec(dllexport)
#else
#define EXPORT_DLL
#endif // 

extern "C"
{
	typedef void(*OnNetMsgCallBack)(void* csObj, void* data, int len);
}

class NativeTCPClient : public EasyTcpClient
{
public:
	//响应网络消息
	virtual void OnNetMsg(netmsg_DataHeader* header)
	{
		if (_callBack)
			_callBack(_csObj, header, header->dataLength);
	}

	void setCallBack(void* csObj, OnNetMsgCallBack cb)
	{
		_csObj = csObj;
		_callBack = cb;
	}
private:
	void* _csObj = nullptr;
	OnNetMsgCallBack _callBack = nullptr;
};



#endif //_CPP_NET_100_DLL_H_