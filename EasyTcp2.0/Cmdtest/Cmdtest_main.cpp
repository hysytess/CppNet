#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<algorithm>
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
public:
	static CellConfig& Create()
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
//				printf("%s %s\n",argv[i],argv[i+1]);
				i++;
			}
		}
//		std::cout<<_cmdMap["-ip"]<<std::endl
//				<<_cmdMap["-port"]<<std::endl
//				<<_cmdMap["-threadcount"]<<std::endl
//				<<_cmdMap["-clientcount"]<<std::endl;
		
	}
	
//	ip=127.0.0.1 
	void madeCmd(char* cmd)
	{
//		ip=127.0.0.1
		char* val = strchr(cmd,'=');
		if(val)
		{
//			val:=127.0.0.1
			*val = '\0';
//			cmd:ip\0
//			val:\0127.0.0.1
			val++;
//			127.0.0.1
			_cmdMap[cmd] = val;
			printf("-%s %s\n",cmd,val);
		}
		else
		{
			_cmdMap[cmd] = "";
		}	
	}
private:
	std::string _binPath;
	std::string _cmd;
	
	std::map<std::string,std::string>_cmdMap;
};

int main(int argc,char* argv[])
{

	//exename ip atoi(port)<=65535 clientcount threadcount
	std::regex ip_regex("((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})(\\.((2(5[0-5]|[0-4]\\d))|[0-1]?\\d{1,2})){3}");
	CellConfig::Create().Init(argc,argv);

	return 0;
}
