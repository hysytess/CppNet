#include "MyClient.hpp"

int main(int argc, char* argv[])
{
	CellStream byteStream;
	byteStream.Write<int8_t>(8);
	byteStream.Write<int16_t>(8);
	byteStream.Write<int32_t>(8);
	byteStream.Write<int64_t>(8);
	byteStream.WriteInt8(8);
	byteStream.WriteInt16(8);
	byteStream.WriteInt32(8);
	byteStream.WriteInt64(8);
	byteStream.WriteInt32(8.0);
	byteStream.WriteInt32(8.0f);

	byteStream.Write<double>(8.0f);
	byteStream.Write<float>(8.0);
	byteStream.Write(8.0); // float
	byteStream.WriteFloat(8.0);
	byteStream.WriteDouble(8.0);

	char str[]{"Hello."};
	int pos[2]{ 1,2 };
	byteStream.WriteArray(&str, sizeof(str));
	byteStream.WriteArray(&pos, sizeof(pos));

	MyClient client;
	CellLog::Instance().setLogPath("BitStreamForClientGoLog.txt", "w");
	client.Connect("127.0.0.1", 4567);
	while(client.isRun())
	{
		client.OnRun();
		CellThread::Sleep(10);
	}
	return 0;
}