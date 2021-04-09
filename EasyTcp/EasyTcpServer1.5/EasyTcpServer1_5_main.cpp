#include "EasyTcpServer1_5.hpp"

bool g_isrun = true;

void Getcmd();

int main(int argc, char** argv) {
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 5566);
	server.Listen(5);

	thread t1(Getcmd);
	t1.detach();

	while (g_isrun&&server.isRun()) {
		server.OnRun();
		// Select task space...deal some thing
	}
	server.CloseScok();
	return 0;
}

void Getcmd() {
	char cmdbuf[128]{};
	while (true) {
		cin >> cmdbuf;
		if (!strcmp(cmdbuf, "exit"))
		{
			g_isrun = false;
			break;
		}
	}
}