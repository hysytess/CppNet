#ifndef _CELL_CONFIG_HPP_
#define _CELL_CONFIG_HPP_

#include "PublicLib.hpp"

#include<regex>
#include<map>


class CellConfig
{
private:
	CellConfig()
	{

	}
	~CellConfig()
	{

	}
	//	ip=127.0.0.1 
	void madeCmd(char* cmd)
	{
		//		ip=127.0.0.1
		char* val = strchr(cmd, '=');
		if (val)
		{
			//			val:=127.0.0.1
			*val = '\0';
			//			cmd:ip\0
			//			val:\0127.0.0.1
			val++;
			//			127.0.0.1
			_cmdMap[cmd] = val;
			printf("-%s %s\n", cmd, val);
		}
		else
		{
			_cmdMap[cmd] = "";
		}
	}
public:
	static CellConfig& Instance()
	{
		static CellConfig obj;
		return obj;
	}

	void Init(int argc, char* argv[])
	{
		_binPath = argv[0];
		// 
		if( 0 < argc &&(!((argc-1)%2)) )
		{
			for(int i = 1;i<argc-1; i++)
			{
                _cmdMap[argv[i]] = argv[i+1];
				//printf("%s %s\n",argv[i],argv[i+1]);
				i++;
			}
		}
//		std::cout<<_cmdMap["-ip"]<<std::endl
//				<<_cmdMap["-port"]<<std::endl
//				<<_cmdMap["-threadcount"]<<std::endl
//				<<_cmdMap["-clientcount"]<<std::endl;
		
	}

	const char* getArgs()
	{
		return getIp();
	}

	int getArgs(const char* argv)
	{
		std::string str = "-port";
		if (!str.compare(argv))
			return getPort();

		str = "-threadcount";
		if (!str.compare(argv))
			return getThreadCount();

		str = "-clientcount";
		if (!str.compare(argv))
			return getClientCount();
		
		str = argv;
			return get_Intargs(str);
		
	}
	int getArgs(const char* str, int def)
	{
		std::string argv = str;
		return get_BuffSize(argv, def);
	}
	
private:
	const char* getIp()
	{
		std::regex ip_regex("((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})(\\.((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})){3}");
		if (_cmdMap.count("-ip"))
		{
			if (!_cmdMap["-ip"].empty())
			{
				if (std::regex_match(_cmdMap["-ip"], ip_regex))
				{
					return _cmdMap["-ip"].c_str();
				}
				CellLog::Error("IP<%s> address error.\n", _cmdMap["-ip"].c_str());
				return nullptr;
			}
		}
		return nullptr;
	}

	unsigned short getPort()
	{
		if (_cmdMap.count("-port"))
		{
			if (!_cmdMap["-port"].empty() && 65535 >= std::stoi(_cmdMap["-port"]) && 0 < std::stoi(_cmdMap["-port"]))
			{
				return (unsigned short)std::stoi(_cmdMap["-port"]);
			}
			CellLog::Error("port<%s> error. -port:1-65535.\n", _cmdMap["-port"].c_str());
			return 4567;
		}
		return 4567;
	}

	int getThreadCount()
	{
		if (_cmdMap.count("-threadcount"))
		{
			if (!_cmdMap["-threadcount"].empty() && 0 < std::stoi(_cmdMap["-threadcount"]))
			{
				return std::stoi(_cmdMap["-threadcount"]);
			}
			CellLog::Error("-threadcount<%d> error. -threadcount:1-1024.\n", stoi(_cmdMap["-threadcount"]));
			return 4;
		}
		return 4;
	}

	unsigned short getClientCount()
	{
		if (_cmdMap.count("-clientcount") && 0 < std::stoi(_cmdMap["-clientcount"]))
		{
			if (!_cmdMap["-clientcount"].empty())
			{
				return (unsigned short)std::stoi(_cmdMap["-clientcount"]);
			}
			CellLog::Error("-clientcount<%s> error. -clientcount:1-10000.\n", _cmdMap["-clientcount"].c_str());
			return 8;
		}
		return 8;
	}

	int get_Intargs(std::string& args)
	{
		if (_cmdMap.count(args))
		{
			if (!_cmdMap[args].empty() && 0 < std::stoi(_cmdMap[args]))
			{
				return std::stoi(_cmdMap[args]);
			}
			CellLog::Error("%s<%s> error. -%s:1-65535.\n", args.c_str(), args.c_str(), _cmdMap[args].c_str());
			return 1;
		}
		CellLog::Error("%s error. %s:1-65535.\n", args.c_str(), args.c_str());
		return 1;
	}

	int get_BuffSize(std::string& args, int def)
	{
		if (_cmdMap.count(args))
		{
			if (!_cmdMap[args].empty()&& 0 < std::stoi(_cmdMap[args]))
			{
				return std::stoi(_cmdMap[args]);
			}
			CellLog::Error("-%s<%d> error. %s:0-102400.\n",_cmdMap[args].c_str(),std::stoi(_cmdMap[args]), _cmdMap[args].c_str());
			return def;
		}
		return def;
	}

private:
	std::string _binPath;
	std::string _cmd;
	
	std::map<std::string,std::string>_cmdMap;
};
#endif
