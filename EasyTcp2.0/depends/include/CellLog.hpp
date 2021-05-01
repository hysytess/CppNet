#ifndef _CELLLOG_HPP_
#define _CELLLOG_HPP_

#include "PublicLib.hpp"
#include "CELLTask.hpp"
#include <ctime>

class CellLog
{
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

	void setLogPath(const char* logPatch, const char* openMode)
	{
		if (_logFile)
		{
			fclose(_logFile);
			_logFile = nullptr;
		}

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

	static void Info(const char* pStr)
	{
		CellLog* pLog = &Instance();
		pLog->_taskServer.addTask([=]() {
			if (pLog->_logFile)
			{
				std::time_t t = std::time(nullptr);
				std::tm* now = std::localtime(&t);
				fprintf(pLog->_logFile,"[%d-%d-%d %d:%d:%d]", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				fprintf(pLog->_logFile, "%s", pStr);
				fflush(pLog->_logFile);
			}
		});
		printf("%s", pStr);

	}
	
	template<typename ...Args>
	static void Info(const char* pformat, Args ... args)
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