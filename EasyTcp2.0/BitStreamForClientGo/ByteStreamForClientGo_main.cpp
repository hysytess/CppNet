#include "MyClient.hpp"

void testCellStream()
{
	// 字节流是有序的 [写入读取位置要对应]
	// 写入 -3>c -2>b -1>a 缓冲区队列[cba->] 出队顺序:a[1] b[2] c[3]
	// 读取(接收顺序) -1>a -2>b -3>c
	CellStream byteStream;
	byteStream.WriteInt8(5);
	byteStream.WriteInt16(6);
	byteStream.WriteInt32(7);
	byteStream.WriteInt64(8);

	byteStream.WriteFloat(14.0);
	byteStream.WriteDouble(15.0);

	char str[]{ "Hello." };
	char str0[5] = "abc";
	int pos0[]{ 5,6,7 };
	int pos[2]{ 1,2 };
	byteStream.WriteArray(str, strlen(str));
	byteStream.WriteArray(pos, 2);

	byteStream.WriteArray(str0, strlen(str0));
	byteStream.WriteArray(pos0, 3);

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

}

int main(int argc, char* argv[])
{
	
	testCellStream();


	CellSendStream byteStream;

	byteStream.setNetCmd(CMD_LOGOUT);

	byteStream.WriteInt8(5);
	byteStream.WriteInt16(6);
	byteStream.WriteInt32(7);
	byteStream.WriteInt64(8);

	byteStream.WriteFloat(14.0);
	byteStream.WriteDouble(15.0);

	char str[]{ "client." };
	char str0[5] = "abc";
	int pos0[]{ 5,6,7 };
	int pos[2]{ 1,2 };
	byteStream.WriteArray(str, strlen(str));
	byteStream.WriteArray(pos, 2);

	byteStream.WriteString(str0);
	byteStream.WriteArray(pos0, 3);
	byteStream.finsh();

	CellLog::Instance().setLogPath("BitStreamForClientGoLog.txt", "w");

	MyClient client;
	client.Connect("127.0.0.1", 4567);

	client.SendData(byteStream.data(),byteStream.length());

	while(client.isRun())
	{
		client.OnRun();
		client.SendData(byteStream.data(), byteStream.length());
		CellThread::Sleep(10);
	}

	return 0;
}