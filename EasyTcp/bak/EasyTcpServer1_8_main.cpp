// 加入高精度包计数器

#include "EasyTcpServer1_8.hpp"

bool g_bRun = true;

void cmdThread()
{
	while (true)
	{
		char cmdBuf[256];
		scanf("%s", cmdBuf);
		if (!strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			break;
		}
	}
}

int main(int argc, char* argv[])
{
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(4);

	std::thread t(cmdThread);
	t.detach();

	while (g_bRun)
	{
		server.OnRun();
	}
	server.Close();

	return 0;
}