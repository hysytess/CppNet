#include <iostream>
#include <stdlib.h>
#include "Alloctor.h"

using namespace std;

int main(int argc, char* argv)
{
	// C++
	char* data = new char;
	delete data;

	char* data1 = new char[128];
	delete[] data1;

	// C
	char* data2 = (char*)malloc(sizeof(char) * 64);
	free(data2);

	//custom
	char* data3 = (char*)mem_alloc(sizeof(char) * 64);
	mem_free(data3);

	return 0;
}