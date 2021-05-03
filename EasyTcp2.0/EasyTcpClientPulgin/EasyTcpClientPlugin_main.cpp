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
	
	EXPORT_DLL void* CellClient_Create(void* csObj, OnNetMsgCallBack cbfun, int sendSize = 10240, int recvSize = 10240)
	{
		NativeTCPClient* pClient = new NativeTCPClient();
		pClient->setCallBack(csObj, cbfun);
		pClient->InitSocket(sendSize, recvSize);
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

	EXPORT_DLL int CellClient_SendStream(NativeTCPClient* pClient, CellWriteStream* wStream)
	{
		if (pClient && wStream)
		{
			wStream->finsh();
			return pClient->SendData(wStream->data(), wStream->length());
		}
		return 0;
	}

	/////////////////////////////////Bytes stream/////////////////////////////
	/////////////////////////////////Write stream/////////////////////////////
	EXPORT_DLL void* CellWriteStream_Create(int nSize)
	{
		CellWriteStream* pWritestream = new CellWriteStream(nSize);
		return pWritestream;
	}

	EXPORT_DLL bool CellWriteStream_WriteInt8(CellWriteStream* wStream, int8_t n)
	{
		if (wStream)
		{
			return wStream->WriteInt8(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStream_WriteInt16(CellWriteStream* wStream, int16_t n)
	{
		if (wStream)
		{
			return wStream->WriteInt16(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStream_WriteInt32(CellWriteStream* wStream,int32_t n)
	{
		if (wStream)
		{
			return wStream->WriteInt32(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStream_WriteInt64(CellWriteStream* wStream, int64_t n)
	{
		if (wStream)
		{
			return wStream->WriteInt64(n);
		}
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteUInt8(CellWriteStream* wStream, uint8_t n)
	{
		if (wStream)
		{
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStream_WriteUInt16(CellWriteStream* wStream, uint16_t n)
	{
		if (wStream)
		{
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStream_WriteUInt32(CellWriteStream* wStream, uint32_t n)
	{
		if (wStream)
		{
			return wStream->Write(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStream_WriteUInt64(CellWriteStream* wStream, uint64_t n)
	{
		if (wStream)
		{
			return wStream->Write(n);
		}
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteFloat(CellWriteStream* wStream, float n)
	{
		if (wStream)
		{
			return wStream->WriteFloat(n);
		}
		return false;
	}
	EXPORT_DLL bool CellWriteStream_WriteDouble(CellWriteStream* wStream, double n)
	{
		if (wStream)
		{
			return wStream->WriteDouble(n);
		}
		return false;
	}

	EXPORT_DLL bool CellWriteStream_WriteString(CellWriteStream* wStream, char* arr)
	{
		if (wStream)
		{
			return wStream->WriteString(arr);
		}
		return false;
	}

	/////////////////////////////////Read stream/////////////////////////////
	EXPORT_DLL void* CellReadStream_Create(char* data, int len)
	{
		CellReadStream* pReadstream = new CellReadStream(data, len);
		return pReadstream;
	}

	EXPORT_DLL int8_t CellReadStream_ReadInt8(CellReadStream* pReadStream)
	{
		if (pReadStream)
			return pReadStream->ReadInt8();
		return 0;
	}
	EXPORT_DLL int16_t CellReadStream_ReadInt16(CellReadStream* pReadStream)
	{
		if (pReadStream)
			return pReadStream->ReadInt16();
		return 0;
	}
	EXPORT_DLL int32_t CellReadStream_ReadInt32(CellReadStream* pReadStream)
	{
		if (pReadStream)
			return pReadStream->ReadInt32();
		return 0;
	}
	EXPORT_DLL int64_t CellReadStream_ReadInt64(CellReadStream* pReadStream)
	{
		if (pReadStream)
			return pReadStream->ReadInt64();
		return 0;
	}

	EXPORT_DLL uint8_t CellReadStream_ReadUInt8(CellReadStream* pReadStream)
	{
		if (pReadStream)
			return pReadStream->ReadUInt8();
		return 0;
	}
	EXPORT_DLL uint16_t CellReadStream_ReadUInt16(CellReadStream* pReadStream)
	{
		if (pReadStream)
			return pReadStream->ReadUInt16();
		return 0;
	}
	EXPORT_DLL uint32_t CellReadStream_ReadUInt32(CellReadStream* pReadStream)
	{
		if (pReadStream)
			return pReadStream->ReadUInt32();
		return 0;
	}
	EXPORT_DLL uint64_t CellReadStream_ReadUInt64(CellReadStream* pReadStream)
	{
		if (pReadStream)
			return pReadStream->ReadUInt64();
		return 0;
	}

	EXPORT_DLL float CellReadStream_ReadFloat(CellReadStream* pReadStream)
	{
		if (pReadStream)
			return pReadStream->ReadFloat();
		return 0;
	}
	EXPORT_DLL double CellReadStream_ReadDouble(CellReadStream* pReadStream)
	{
		if (pReadStream)
			return pReadStream->ReadDouble();
		return 0;
	}

	EXPORT_DLL bool CellReadStream_ReadString(CellReadStream* pReadStream, char* buffer, int len)
	{
		if (pReadStream && buffer)
		{
			return pReadStream->ReadArray(buffer, len);
		}
		return false;
	}
	EXPORT_DLL uint32_t CellReadStream_OnlyReadUint32(CellReadStream* pReadStream)
	{
		uint32_t len = 0;
		if (pReadStream)
		{
			return pReadStream->onlyRead(len);
		}
		return len;
	}
}
