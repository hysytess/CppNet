// 面向对象封装Client

#include "MSGPROC.hpp"
#include "EasyTcpClient1_5.hpp"
void GetCmd(EasyTcpClient* client);
int main(int argc, char** argv) {
	EasyTcpClient client;
	client.CreateSocket();
	char ip[16] = "192.168.80.1";
	if (client.ConnectToHost(ip, 5566, client.getSock()));
	else exit(1);

	std::thread t1(GetCmd, &client);
	t1.detach();

	while (g_isrun&&client.OnRun())
	{
		Message facebackmsg{};
		// strcpy(facebackmsg.msg, "hello!,i'm client.");
		// client.sendToHost(&facebackmsg);
		// Select space ... deal some business
	}

#ifdef _WIN32
	system("pause");
#endif
	return 0;
}

void GetCmd(EasyTcpClient* client) {
	char cmdbuf[128]{};
	while (true) {
		memset(cmdbuf, 0, 128);
		cin >> cmdbuf;
		if (!strcmp(cmdbuf, "exit"))
		{
			g_isrun = false;
			break;
		}
		if (!strcmp(cmdbuf, "login")) {
			Login login{};
			strcpy(login.UserName, "User123");
			strcpy(login.PassWord, "password123");
			client->sendToHost(&login);
		}
		if (!strcmp(cmdbuf, "logout")) {
			Loginout logout{};
			strcpy(logout.UserName, "User123");
			client->sendToHost(&logout);
		}
		if (!strcmp(cmdbuf, "message")) {
			Message info;
			cin >> info.msg;
			client->sendToHost(&info);
		}
	}
}