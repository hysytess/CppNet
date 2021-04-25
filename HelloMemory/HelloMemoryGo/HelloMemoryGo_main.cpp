#include "Alloctor.h"
#include <stdlib.h>

int main(int argc, char* argv)
{
	// C++
	//char* data = new char;
	//delete data;

	//char* data1 = new char[128];
	//delete[] data1;

	//char* data2 = new char[64];
	//delete[] data2;

	char* data3[1100];
	for (size_t i = 0; i < 1100; i++)
	{
		//data3[i] = new char[rand()%1024 + 1];
		data3[i] = new char[i + 1];
	}

	for (size_t i = 0; i < 1100; i++)
	{
		delete[] data3[i];
	}
	return 0;
}