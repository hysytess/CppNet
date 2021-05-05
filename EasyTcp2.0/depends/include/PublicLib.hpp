#ifndef _PUBLICLIB_HPP_
#define _PUBLICLIB_HPP_

#ifdef _WIN32
#define FD_SETSIZE      65535
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>
#include <signal.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#define SEND_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE
#define _Cell_THREAD_CONT 4

#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "CELLTask.hpp"
#include "CellLog.hpp"
#include "CellConfig.hpp"

#ifdef _DEBUG
#define Info(...) Debug(__VA_ARGS__)
#else
#define Info(...) Info(__VA_ARGS__)
#endif

#include<stdio.h>
#include <string>
#include <iostream>
#include <algorithm>

#endif