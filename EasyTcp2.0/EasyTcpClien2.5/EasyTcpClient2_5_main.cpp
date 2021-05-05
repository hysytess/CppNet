// v2.5支持命令脚本
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
			CellLog::Info("exit.\n");
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

void sendThread(int id,const char* ip,unsigned short port)
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
		if (SOCKET_ERROR != client[i]->Connect(ip, port))
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

const char* ip = "127.0.0.1";
int nPort = 0;
int nThread = 0;
int nClient = 0;

int main(int argc, char* argv[])
{
	CellLog::Instance().setLogPath("client",nullptr,"w");
	CellConfig::Instance().Init(argc, argv);
	ip = CellConfig::Instance().getStr("strIp", "127.0.0.1");
	nPort = CellConfig::Instance().getInt("nPort", 4567);
	nThread = CellConfig::Instance().getInt("nThread", 1);
	nClient = CellConfig::Instance().getInt("nClient", 100);
	if (!ip)
		CellLog::Info("<Any:%d>, thread=%d, clientcount=%d\n", nPort, nThread, nClient);
	else
		CellLog::Info("<%s:%d>, thread=%d, clientcount=%d\n", ip, nPort, nThread, nClient);


	std::thread t(cmdThread);
	t.detach();

	for (int n = 0; n < nThread; n++)
	{
		std::thread clientThread(sendThread, n, ip, nPort);
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


	return 0;
}