
#include "MyServer2_0.hpp"

int main(int argc, char* argv[])
{
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

	std::cout << "Exit" << std::endl;

	return 0;
}