#include "Alloctor.h"
#include "HelloObjectPool.hpp"
#include <stdio.h>

class ClassA :public ObjectPoolBase<ClassA>
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

int main(int argc, char* argv)
{
	ClassA *a1 = ClassA::createObject();
	ClassA::destroyObject(a1);

	ClassA *a2 = ClassA::createObject(2);
	ClassA::destroyObject(a2);

	ClassA *a3 = ClassA::createObject(2, 3);
	ClassA::destroyObject(a3);
	return 0;
}