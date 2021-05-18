// version2.6 优化

#include "MyServer2_5.hpp"
#include "../depends/include/CellConfig.hpp"

int main(int argc, char* args[])
{
	CellConfig::Instance().Init(argc,args);
	const char* strIP = CellConfig::Instance().getStr("strIP","any");
	uint16_t nPort = CellConfig::Instance().getInt("nPort", 4567);
	uint16_t nThread = CellConfig::Instance().getInt("nThread", 4);
	uint16_t nClient = CellConfig::Instance().getInt("nClient",10000);

	if (0 == strcmp("any",strIP))
		strIP = nullptr;
	

	CellLog::Instance().setLogPath("serverLog2_5", "w");
	MyServer server;
	server.InitSocket();
	server.Bind(strIP, nPort);
	server.Listen(64);
	server.Start(nThread);

	while (true)
	{
		char cmdBuf[256];
		scanf("%s", cmdBuf);
		if (!strcmp(cmdBuf, "exit"))
		{
			server.Close();
			break;
		}
	}

	CellLog_Debug("exit.");

	std::chrono::milliseconds t(100);
	std::this_thread::sleep_for(t);
	return 0;
}