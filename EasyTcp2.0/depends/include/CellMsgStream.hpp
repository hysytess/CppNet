#ifndef _CELL_MSG_STREAM_HPP_
#define _CELL_MSG_STREAM_HPP_

#include "../depends/include/CellStream.hpp"

//Receive message byte stream
class CellRecvStream:public CellStream
{
public:
	CellRecvStream(netmsg_DataHeader* header)
		:CellStream((char*)header,header->dataLength)
	{
		push(header->dataLength);
		// 预读取消息长度数据 抛掉(占位符) 
		// 或由客户{MyClient:public EasyTcpClient,MyServer:public EasyTcpServer}读取
		//ReadInt16();
		//getNetCmd();
	}
	uint16_t getNetCmd()
	{
		uint16_t cmd = CMD_ERROR;
		Read<uint16_t>(cmd);
		return cmd;
	}
};

//Send message byte stream
class CellSendStream :public CellStream
{
public:
	//	用户自定义的流缓冲区:外部缓冲区地址  外部缓冲区大小  是否需要自动释放
	CellSendStream(char* pData, int nSize = 1024, bool bDelete = false)
		:CellStream(pData, nSize, bDelete)
	{
		// 占位
		Write<uint16_t>(0);
	}

	CellSendStream(int nSize = 1024)
		:CellStream(nSize)
	{
		// 占位
		Write<uint16_t>(0);
	}

	void setNetCmd(uint16_t cmd)
	{
		Write<uint16_t>(cmd);
	}

	// 直接写到流缓冲区队列头
	void finsh()
	{
		int pos = getWritePos();
		setWritePos(0);
		Write<uint16_t>(pos);
		setWritePos(pos);
	}
};
#endif
