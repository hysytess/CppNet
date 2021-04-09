#ifndef MSGPROC_HPP
#define MSGPROC_HPP

#include <vector>
#ifdef _WIN32

#define  _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGSs
#define strcpy strcpy_s
#pragma comment(lib,"ws2_32.lib")

#include <windows.h>
#include <WinSock2.h>

#else

#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define SOCKET int
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR   (SOCKET)(-1)
#endif

enum DataPackageType {
	DPT_EXIT = 0,
	DPT_LOGIN,
	DPT_LOGOUT,
	DPT_LOGINOK,
	DPT_LOGOUTOK,
	DPT_LOGINERROR,
	DPT_LOGOUTERROR,
	DPT_MSG,
	DPT_NEWUSER_JOIN,
	DPT_ERROR
};

struct DataHeader {
	short dataLength;
	short cmd;
};

struct Login : public DataHeader {
	Login() {
		dataLength = sizeof(Login);
		cmd = DPT_LOGIN;
	}
	char UserName[32];
	char PassWord[32];
};
struct Loginout : public DataHeader {
	Loginout() {
		dataLength = sizeof(Loginout);
		cmd = DPT_LOGOUT;
	}
	char UserName[32];
};
struct LoginResult : public DataHeader {
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = DPT_LOGINOK;
	}
	int result;
};
struct LoginoutResult : public DataHeader {
	LoginoutResult() {
		dataLength = sizeof(LoginoutResult);
		cmd = DPT_LOGOUTOK;
	}
	int result;
};
struct Message : public DataHeader {
	Message() {
		dataLength = sizeof(Message);
		cmd = DPT_MSG;
	}
	char msg[126];
};

struct NewUserJoinInfo :public DataHeader {
	NewUserJoinInfo() {
		dataLength = sizeof(NewUserJoinInfo);
		cmd = DPT_NEWUSER_JOIN;
	}
	int socketId;
};

std::vector<SOCKET> g_clientList;

bool g_isrun = true;

#endif // MSGPROC_HPP