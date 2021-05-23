// version2.6 支持命令行脚本 性能优化

#include "MyClient.hpp"
#include "../depends/include/CellThread.hpp"

void WorkThread(CellThread* pThread, int id)
{
	std::vector<MyClient*> clients(nClient);
	int begin = 0;
	int end = nClient;
	for (int n = begin; n < end; n++)
	{
		if (!pThread->isRun())
			break;
		clients[n] = new MyClient();
		CellThread::Sleep(0);
	}
	for (int n = begin; n < end; n++)
	{
		if (!pThread->isRun())
			break;
		if (INVALID_SOCKET == clients[n]->InitSocket(nSendBuffSize, nRecvBuffSize))
			break;
		if (SOCKET_ERROR == clients[n]->Connect(strIP, nPort))
			break;
		nConnect++;

		CellThread::Sleep(0);
	}
	CellLog_Debug("thread<%d>,connect<begin=%d, end=%d, nconnect=%d>", id, begin, end, (int)nConnect);

	readyCount++;
	while (readyCount < nThread && pThread->isRun())
	{
		CellThread::Sleep(10);
	}

	netmsg_Login login;
	strcpy(login.userName, "lyd");
	strcpy(login.PassWord, "lydmm");

	auto t2 = CELLTime::getNowInMillisec();
	auto t0 = t2;
	auto dt = t0;
	CELLTimestamp tTime;
	while (pThread->isRun())
	{
		t0 = CELLTime::getNowInMillisec();
		dt = t0 - t2;
		t2 = t0;

		{
			int count = 0;
			for (int m = 0; m < nMsg; m++)
			{
				for (int n = begin; n < end; n++)
				{
					if (clients[n]->isRun())
					{
						if (clients[n]->SendTest(&login) > 0)
							++sendcnt;
					}
				}
			}
			for (int n = begin; n < end; n++)
			{
				if (clients[n]->isRun())
				{
					if (!clients[n]->OnRun(0))
					{
						nConnect--;
						continue;
					}
					clients[n]->checkSend(dt);
				}
			}
		}
		CellThread::Sleep(nWorkSleep);
	}
	
	for (int n = begin; n < end; n++)
	{
		clients[n]->Close();
		delete clients[n];
	}
	CellLog_Debug("thread<%d>,exit.", id);
}


int main(int argc, char* args[])
{
	CellLog::Instance().setLogPath("ClientLog2_6","w",false);

	CellConfig::Instance().Init(argc, args);
	strIP = CellConfig::Instance().getStr("strIP", "127.0.0.1");
	nPort = CellConfig::Instance().getInt("nPort", 4567);
	nThread = CellConfig::Instance().getInt("nThread", 4);
	nClient = CellConfig::Instance().getInt("nClient", 2500);
	nMsg = CellConfig::Instance().getInt("nMsg", 10);
	nSendSleep = CellConfig::Instance().getInt("nSendSleep", 100);
	nSendBuffSize = CellConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
	nRecvBuffSize = CellConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);

	CellThread tCmd;
	tCmd.Start(nullptr, [](CellThread* pThread) {
		while (pThread->isRun())
		{
			char cmdBuf[256]{};
			scanf("%s", cmdBuf);
			if (!strcmp(cmdBuf, "exit"))
			{
				//pThread->Exit();
				CellLog_Debug("exit.");
				break;
			}
		}
	});

	// 启动模拟客户端线程
	std::vector<CellThread*> threads;
	for (int n = 0; n < nThread; n++)
	{
		CellThread* t = new CellThread();
		t->Start(nullptr, [n](CellThread* pThread) {
			WorkThread(pThread, n + 1);
		});
		threads.push_back(t);
	}

	CELLTimestamp tTime;
	while (tCmd.isRun())
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			CellLog_Debug("thread<%d>,clients<%d>,connect<%d>,time<%lf>,send<%d>", nThread, nClient*nThread, (int)nConnect, t, (int)sendcnt);
			sendcnt = 0;
			tTime.update();
		}
		CellThread::Sleep(1);
	}

	for (auto t : threads)
	{
		t->Close();
		delete t;
	}
	return 0;
}