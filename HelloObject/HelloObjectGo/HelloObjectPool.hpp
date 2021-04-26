#ifndef _HELLOOBJECTPOOL_HPP_
#define _HELLOOBJECTPOOL_HPP_

#include <stdlib.h>

template<class Type>
class ObjectPoolBase
{
public:
	void* operator new(size_t nSize)
	{
		return malloc(nSize);
	}

	void operator delete(void *p)
	{
		free(p);
	}
	// 可變參函數模板
	template<typename ...Args>
	static Type* createObject(Args...args)
	{
		Type *obj = new Type(args...);
		return obj;
	}
	static void destroyObject(Type* obj)
	{
		delete obj;
	}
};

#endif