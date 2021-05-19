#ifndef _CELL_NET_WORK_HPP_
#define _CELL_NET_WORK_HPP_

#include "PublicLib.hpp"

class CellNetWork
{
private:
	CellNetWork()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif

#ifndef _WIN32
		// 屏蔽系统终止信号

		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			return;
#endif

	}
	~CellNetWork()
	{
#ifdef _WIN32
		WSACleanup();
#endif
	}
public:
	static void Init()
	{
		static CellNetWork obj;
	}
	
	static int make_reuseaddr(SOCKET fd)
	{
		int flag = 1;
		if (SOCKET_ERROR == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag)))
		{
			CellLog_Error("setsockopt SO_REUSEADDR fail.");
			return SOCKET_ERROR;
		}
		return 1;
	}
};

#endif
