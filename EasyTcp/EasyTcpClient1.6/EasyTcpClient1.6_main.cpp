#include "EasyTcpClient1_6.hpp"
#include <thread>

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256]{};
		scanf("%s", cmdBuf);
		if (!strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("exit.\n");
			break;
		}
	}
}

int main(int argc, char* argv[])
{
	const int cCount = 1500;//FD_SETSIZE - 1;

	EasyTcpClient *client[cCount];
	for (int i = 0; i < cCount; i++) {
		client[i] = new EasyTcpClient();
	}

	int client_cnt = 1;
	for (int i = 0; i < cCount; i++) {
		if (SOCKET_ERROR != client[i]->Connect("192.168.80.1", 4567))
		{
			printf("client<%d> connected.\n", client_cnt);
			client_cnt++;
		}
	}
	client_cnt = 0;

	std::thread t1(cmdThread);
	t1.detach();

	Login login;
	strcpy(login.userName, "lyd");
	strcpy(login.PassWord, "lydmm");
	while (g_bRun)
	{
		for (int i = 0; i < cCount; i++) {
			client[i]->SendData(&login);
			client[i]->OnRun();
		}
		// printf("空闲时间处理其他任务...\n");
	}

	for (int j = 0; j < cCount; j++)
		client[j]->Close();

	return 0;
}