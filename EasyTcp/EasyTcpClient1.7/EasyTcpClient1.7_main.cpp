#include "EasyTcpClient1_7.hpp"
#include <thread>
#include <atomic>
#include <chrono>

std::atomic_int client_cnt = 1;
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

const int cCount = 10000;//FD_SETSIZE - 1;
const int tCount = 4;
EasyTcpClient *client[cCount];

void sendThread(int id)
{
	int start = id * (cCount / tCount);
	int end = (id + 1)*(cCount / tCount);

	for (int i = start; i < end; i++) {
		if (!g_bRun) return;
		client[i] = new EasyTcpClient();
	}

	for (int i = start; i < end; i++)
	{
		if (!g_bRun) return;
		if (SOCKET_ERROR != client[i]->Connect("192.168.80.1", 4567))
		{
			std::cout << "thread<" << id << ">,client<" << client_cnt << "> connected.\n";
			client_cnt++;
		}
	}

	std::chrono::milliseconds t(3000);
	std::this_thread::sleep_for(t);

	const int packageCnt = 5;
	Login login[packageCnt];
	for (int n = 0; n < packageCnt; n++)
	{
		strcpy(login[n].userName, "lyd");
		strcpy(login[n].PassWord, "lydmm");
	}

	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int i = start; i < end; i++) {
			client[i]->SendData(login, nLen);
			client[i]->OnRun();
		}
		// printf("空闲时间处理其他任务...\n");
	}

	for (int j = start; j < end; j++)
	{
		client[j]->Close();
		delete client[j];
	}
}

int main(int argc, char* argv[])
{
	std::thread t(cmdThread);
	t.detach();

	for (int n = 0; n < tCount; n++)
	{
		std::thread clientThread(sendThread, n);
		clientThread.detach();
	}
	while (g_bRun);
	Sleep(100);

	return 0;
}