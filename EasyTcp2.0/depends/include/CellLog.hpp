#ifndef _CELLLOG_HPP_
#define _CELLLOG_HPP_

#include "PublicLib.hpp"
#include "CELLTask.hpp"
#include <ctime>

class CellLog
{
	// Info
	// Debug
	// Warning
	// Error

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
	//const char* logName: File Name 
	//const char* logPatch : File patch 
	//const char* openMode: OpenMode
	void setLogPath(const char* logName = nullptr,const char* logPatch = nullptr, const char* openMode="w")
	{
		if (_logFile)
		{
			fclose(_logFile);
			_logFile = nullptr;
		}

		//自定义 路径
		if (logPatch) 
		{
			_logFile = fopen(logPatch, openMode);
			if (_logFile)
			{
				//Info("CellLog::setFilePath <%s,%s>",logPatch,openMode);
			}
			else
			{
				Info("Error,CellLog::setFilePath <%s,%s> fail.", logPatch, openMode);
			}
		}
		else //默认
		{
			static char logfilepath[256]{};
			std::time_t t = std::time(nullptr);
			std::tm* now = std::localtime(&t);
			int mon = now->tm_mon + 1;
			if (mon > 12)
				mon = 1;
			sprintf(logfilepath, "%s[%d-%d-%d]log.txt",logName,now->tm_year + 1900, mon, now->tm_mday);
			printf("%s\n", logfilepath);
			_logFile = fopen(logfilepath, openMode);
			if (_logFile)
			{
				//Info("CellLog::setFilePath <%s,%s>",logPatch,openMode);
			}
			else
			{
				Info("Error,CellLog::setFilePath <%s,%s> fail.", logPatch, openMode);
			}
		}


	}

	static void Error(const char* pStr)
	{
		Error("###Error %s", pStr);
	}
	template<typename ...Args>
	static void Error(const char* pformat, Args ... args)
	{
		Echo("Error", pformat, args...);
	}

	static void Debug(const char* pStr)
	{
		Debug("Debug%s", pStr);
	}
	template<typename ...Args>
	static void Debug(const char* pformat, Args ... args)
	{
		Echo("Debug", pformat, args...);
	}


	static void Warning(const char* pStr)
	{
		Warning("---Warning %s", pStr);
	}
	template<typename ...Args>
	static void Warning(const char* pformat, Args ... args)
	{
		Echo("---Warning ", pformat, args...);
	}

	static void Info(const char* pStr)
	{
		Info("Info", pStr);
	}
	template<typename ...Args>
	static void Info(const char* pformat, Args ... args)
	{
		Echo("Info",pformat, args...);
	}

	template<typename ...Args>
	static void Echo(const char* type,const char* pformat, Args ... args)
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
				fprintf(pLog->_logFile, "%s",type);
				fprintf(pLog->_logFile, "[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, mon, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, pformat, args...);
				fflush(pLog->_logFile);
			}
		});
		printf(pformat, args...);

	}

private:
	FILE* _logFile = nullptr;
	CellTaskServer _taskServer;
};

#endif 