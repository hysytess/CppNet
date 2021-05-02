#include <string>
#include "depends/include/DllGo_header.h"

extern "C"
{
	int _declspec(dllexport) Add(int a, int b)
	{
		return a + b;
	}

	int _declspec(dllexport) Sub(int a, int b)
	{
		return a - b;
	}
	// C#中的函数
	//public delegate void CallBack1(string s);
    
	//[MonoPInvokeCallback(typeof(CallBack1))]
	//public static void CallBackFun1(string s)
	//{
	//	Debug.Log(s);
	//}
	//...
	//TellCall1("C++&C# for unity!", CallBackFun1);

	// 函数指针 C#交互 回调C#中的函数
	typedef void(*CallBack1)(const char* s);
	//C# TellCall1("World!", CallBackFun1); 中传入参数string:"C++&C# for unity!", 函数对象[回调对象(函数指针)]: CallBackFun1
	void _declspec(dllexport) TellCall1(const char* s, CallBack1 cbfun)
	{
		std::string str = "Hello,";
		str += s;
		// str : "Hello,C++&C# for unity!"
		//回调调用C# 函数 public static void CallBackFun1(string s)
		cbfun(str.c_str());
		//C#: public static void CallBackFun1(string s)
		// s : "Hello,world!"
	}

}


