#ifndef _EASYTCPDLLGO_H_
#define _EASYTCPDLLGO_H_
extern "C"
{
	int _declspec(dllexport) Add(int a, int b);
	int _declspec(dllexport) Sub(int a, int b);
}

#endif
