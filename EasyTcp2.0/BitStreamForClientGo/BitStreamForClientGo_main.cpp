#include "MyClient.hpp"

int main(int argc, char* argv[])
{
	CellStream byteStream;
	byteStream.WriteInt8(5);
	byteStream.WriteInt16(6);
	byteStream.WriteInt32(7);
	byteStream.WriteInt64(8);

	byteStream.WriteFloat(14.0);
	byteStream.WriteDouble(15.0);

	char str[]{"Hello."};
	int pos[2]{ 1,2 };
	byteStream.WriteArray(&str, sizeof(str));
	byteStream.WriteArray(&pos, sizeof(pos));

	auto a1 = byteStream.ReadInt8();
	auto a2 = byteStream.ReadInt16();
	auto a3 = byteStream.ReadInt32();
	auto a4 = byteStream.ReadInt64();
	auto a5 = byteStream.ReadFloat();
	auto a6 = byteStream.ReadDouble();



	//MyClient client;
	//CellLog::Instance().setLogPath("BitStreamForClientGoLog.txt", "w");
	//client.Connect("127.0.0.1", 4567);
	//while(client.isRun())
	//{
	//	client.OnRun();
	//	CellThread::Sleep(10);
	//}

	return 0;
}