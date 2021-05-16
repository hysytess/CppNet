// 添加接收消息子线线程

#include "MyClient.hpp"
#include <thread>
#include <chrono>
#include <atomic>

std::atomic_int client_cnt{};
std::atomic_int sendcnt{};
std::atomic_int readyCount{};

const int cCount = 10000;//FD_SETSIZE - 1;
const int tCount = 4;
EasyTcpClient *client[cCount];
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
			CellLog_Debug("exit.");
			break;
		}
	}
}

void recvThread(int start, int end)
{
	while (g_bRun)
	{
		for (int i = start; i < end; i++)
		{
			if (client[i]->OnRun());
		}
		// printf("空闲时间处理其他任务...\n");
	}
}

void sendThread(int id)
{
	int start = id * (cCount / tCount);
	int end = (id + 1)*(cCount / tCount);

	for (int i = start; i < end; i++)
	{
		if (!g_bRun) return;
		client[i] = new MyClient();
	}

	for (int i = start; i < end; i++)
	{
		if (!g_bRun) return;
		if (SOCKET_ERROR != client[i]->Connect("127.0.0.1", 4567))
		{
			//std::cout << "thread<" << id << ">,client<" << client_cnt << "> connected.\n";
			client_cnt++;
		}
	}

	readyCount++;
	while (readyCount < tCount)
	{
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}

	std::thread t2(recvThread, start, end);
	t2.detach();

	const int packageCnt = 5;
	netmsg_Login login[packageCnt];
	for (int n = 0; n < packageCnt; n++)
	{
		strcpy(login[n].userName, "lyd");
		strcpy(login[n].PassWord, "lydmm");
	}

	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int i = start; i < end; i++)
		{
			if (SOCKET_ERROR != client[i]->SendData(login))
			{
				sendcnt++;
			}
		}
		//std::chrono::milliseconds t(100);
		//std::this_thread::sleep_for(t);
	}

	for (int j = start; j < end; j++)
	{
		client[j]->Close();
		delete client[j];
	}
}

int main(int argc, char* argv[])
{
	CellLog::Instance().setLogPath("ClientLog2_3","w");

	std::thread t(cmdThread);
	t.detach();

	for (int n = 0; n < tCount; n++)
	{
		std::thread clientThread(sendThread, n);
		clientThread.detach();
	}

	CELLTimestamp tTime;
	while (g_bRun)
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			std::cout << "thread<" << readyCount << ">,client<"<< client_cnt <<">,time<"<< t <<">,send<"<< (int)(sendcnt / t) <<">\n";
			sendcnt = 0;
			tTime.update();
		}
	}

	std::chrono::milliseconds t1(100);
	std::this_thread::sleep_for(t1);

	return 0;
}