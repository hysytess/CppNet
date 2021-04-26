// v1.8 加入智能指针
#include "Alloctor.h"
#include <stdio.h>
#include <mutex>

std::mutex m;
const int tCount = 8;
const int mCount = 100000;
const int nCount = mCount / tCount;

void workFun(int index)
{
}

class A
{
public:
	A()
	{
		printf("classA\n");
	}
	~A()
	{
		printf("~classA\n");
	}
	int num;
};

std::shared_ptr<A> fun(std::shared_ptr<A> pA)
{
	pA->num++;
	return pA;
}

int main(int argc, char* argv)
{
	//std::shared_ptr<A> c;
	{
		std::shared_ptr<A> b = std::make_shared<A>();
		b->num = 100;
		printf("%d, use_count=%d\n", b->num, b.use_count());
		//c = fun(b);
		std::shared_ptr<A> c = fun(b);
		printf("%d, use_count=%d\n", b->num, b.use_count());
	}
	//printf("shared_ptr<A> c::use_count=%d\n", c.use_count());
	return 0;
}