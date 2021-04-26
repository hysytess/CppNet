#include "Alloctor.h"
#include "HelloObjectPool.hpp"
#include <stdio.h>
#include <mutex>

std::mutex m;
const int tCount = 4;
const int mCount = 16;
const int nCount = mCount / tCount;

// ClassA 池中有10個對象
class ClassA :public ObjectPoolBase<ClassA, 5>
{
public:
	ClassA()
	{
		printf("ClassA\n");
	}
	ClassA(int n)
	{
		printf("ClassA n1=%d\n", n);
	}
	ClassA(int n1, int n2)
	{
		printf("ClassA n1=%d n2=%d\n", n1, n2);
	}
	~ClassA()
	{
		printf("~ClassA\n");
	}

	int num;
};

// ClassB 池中有10個對象
class ClassB :public ObjectPoolBase<ClassB, 10>
{
public:
	ClassB()
	{
		printf("ClassB\n");
	}
	ClassB(int n)
	{
		printf("ClassB n1=%d\n", n);
	}
	ClassB(int n1, int n2)
	{
		printf("ClassB n1=%d n2=%d\n", n1, n2);
	}
	~ClassB()
	{
		printf("~ClassB\n");
	}

	int num;
};

void workFun(int index)
{
	ClassA* data[nCount];
	for (size_t i = 0; i < nCount; i++)
		data[i] = ClassA::createObject(6);
	for (size_t i = 0; i < nCount; i++)
		ClassA::destroyObject(data[i]);
}

int main(int argc, char* argv)
{
	//ClassA* a1 = ClassA::createObject();
	//ClassA* a2 = ClassA::createObject(6);
	//ClassA* a3 = ClassA::createObject(5, 6);
	//ClassA::destroyObject(a1);
	//ClassA::destroyObject(a2);
	//ClassA::destroyObject(a3);

	//ClassB* b1 = ClassB::createObject();
	//ClassB* b2 = ClassB::createObject(6);
	//ClassB* b3 = ClassB::createObject(5, 6);
	//ClassB::destroyObject(b1);
	//ClassB::destroyObject(b2);
	//ClassB::destroyObject(b3);

	for (int i = 0; i < tCount; i++)
	{
		std::thread t(workFun, i);
		t.join();
	}

	return 0;
}