// version2.4 支持字节流传输

#include "MyServer2_5.hpp"
#include "../depends/include/CellConfig.hpp"

const char* ip = "127.0.0.1";
int nPort = 0;
int nThread = 0;
int nClient = 0;

int main(int argc, char* argv[])
{
	
	CellLog::Instance().setLogPath("server_v2_5", nullptr, "w");

	CellConfig::Instance().Init(argc, argv);
	ip = CellConfig::Instance().getStr("strIp", "127.0.0.1");
	nPort = CellConfig::Instance().getInt("nPort", 4567);
	nThread = CellConfig::Instance().getInt("nThread", 1);
	nClient = CellConfig::Instance().getInt("nClient", 100);
	if (!ip)
		CellLog::Info("<Any:%d>, thread=%d, clientcount=%d\n", nPort, nThread, nClient);
	else
		CellLog::Info("<%s:%d>, thread=%d, clientcount=%d\n", ip, nPort, nThread, nClient);


	MyServer server;
	server.InitSocket();
	server.Bind(ip, nPort); // ip port
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

	CellLog::Info("exit.\n");

	return 0;
}