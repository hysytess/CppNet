#ifndef _CELLLOG_HPP_
#define _CELLLOG_HPP_

#include "PublicLib.hpp"
#include "CELLTask.hpp"
#include <ctime>

class CellLog
{
	// Info
	// Debug
	// Warring
	// Error
#ifdef _DEBUG

#ifndef CellLog_Debug
#define CellLog_Debug(...) CellLog::Debug(__VA_ARGS__)
#endif

#else

#ifndef CellLog_Debug
#define	CellLog_Debug(...) CellLog::Info(__VA_ARGS__)
#endif

#endif // _DEBUG

#define CellLog_Info(...) CellLog::Info(__VA_ARGS__)
#define CellLog_Warring(...) CellLog::Warring(__VA_ARGS__)
#define CellLog_Error(...) CellLog::Error(__VA_ARGS__)


private:
	CellLog()
	{ 
		_taskServer.Start();
	}
	~CellLog()
	{ 
		_taskServer.Close();
		if (_logFile)
		{
			fclose(_logFile);
			_logFile = nullptr;
		}
		
	}
public:
	static CellLog& Instance()
	{
		static CellLog sLog;
		return sLog;
	}

	void setLogPath(const char* LogName, const char* openMode)
	{
		if (_logFile)
		{
			fclose(_logFile);
			_logFile = nullptr;
		}

		static char logPath[256]{};
		auto t = system_clock::now();
		auto tNow = system_clock::to_time_t(t);
		std::tm* now = std::localtime(&tNow);
		int mon = now->tm_mon + 1;
		if (mon > 12)
			mon = 1;
		sprintf(logPath, "%s[%d-%d-%d_%d-%d-%d].txt", LogName, now->tm_year + 1900, mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		_logFile = fopen(logPath, openMode);
		if (_logFile)
		{
			Info("CellLog::setFilePath <%s,%s>", logPath,openMode);
		}
		else
		{
			Info("Error,CellLog::setFilePath <%s,%s> fail.\n", logPath, openMode);
		}
	}
	
	static void Error(const char* pStr)
	{
		Error("%s", pStr);
	}

	template<typename ...Args>
	static void Error(const char* pformat, Args ... args)
	{
		Echo("###Error ", pformat, args...);
	}

	static void Warring(const char* pStr)
	{
		Warring("%s", pStr);
	}

	template<typename ...Args>
	static void Warring(const char* pformat, Args ... args)
	{
		Echo("Warring ", pformat, args...);
	}

	static void Debug(const char* pStr)
	{
		Debug("%s", pStr);
	}

	template<typename ...Args>
	static void Debug(const char* pformat, Args ... args)
	{
		Echo("Debug ", pformat, args...);
	}

	static void Info(const char* pStr)
	{
		Info("%s", pStr);
	}

	template<typename ...Args>
	static void Info(const char* pformat, Args ... args)
	{
		Echo("Info ", pformat, args...);
	}

	template<typename ...Args>
	static void Echo(const char* type, const char* pformat, Args ... args)
	{
		CellLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				std::time_t t = std::time(nullptr);
				std::tm* now = std::localtime(&t);
				int mon = now->tm_mon + 1;
				if (mon > 12)
					mon = 1;
				fprintf(pLog->_logFile, "%s", type);
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, pformat, args...);
				fprintf(pLog->_logFile, "%s", "\n");
				fflush(pLog->_logFile);
			}
			printf(pformat, args...);
			printf("%s", "\n");
		});

	}

private:
	FILE* _logFile = nullptr;
	CellTaskServer _taskServer;
};

#endif 