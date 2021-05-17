#ifndef MSGPROC_HPP
#define MSGPROC_HPP

#include <vector>

enum DataPackageType {
	DPT_EXIT = 0,
	DPT_LOGIN,
	DPT_LOGOUT,
	DPT_LOGINOK,
	DPT_LOGOUTOK,
	DPT_LOGINERROR,
	DPT_LOGOUTERROR,
	DPT_MSG,
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
	char msg[508];
};

std::vector<SOCKET> g_clientList;

#endif // MSGPROC_HPP