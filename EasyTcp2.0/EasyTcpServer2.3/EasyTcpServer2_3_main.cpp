
#include "MyServer2_3.hpp"

int main(int argc, char* argv[])
{
	CellLog::Instance().setLogPath("serverLog.txt", "w");
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(64);
	server.Start(4);

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

	std::chrono::milliseconds t(100);
	std::this_thread::sleep_for(t);
	return 0;
}