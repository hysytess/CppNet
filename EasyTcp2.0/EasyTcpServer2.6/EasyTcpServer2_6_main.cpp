// version2.5 支持命令脚本

#include "MyServer2_6.hpp"
#include "../depends/include/CellConfig.hpp"

int main(int argc, char* args[])
{
	CellLog::Instance().setLogPath("serverLog2_6", "w");

	CellConfig::Instance().Init(argc,args);
	const char* strIP = CellConfig::Instance().getStr("strIP","any");
	uint16_t nPort = CellConfig::Instance().getInt("nPort", 4567);
	uint16_t nThread = CellConfig::Instance().getInt("nThread", 4);

	if (0 == strcmp("any",strIP))
		strIP = nullptr;


	MyServer server;
	server.InitSocket();
	server.Bind(strIP, nPort);
	server.Listen(64);
	//server.Start<EasyTcpServer_Select>(nThread);
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

	return 0;
}