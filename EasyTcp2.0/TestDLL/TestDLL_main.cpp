#include <iostream>
#include <cstdlib>

#include "./depends/include/DllGo_header.h"

int main(int argc, char* argv[])
{
	std::cout << "1+2=" << Add(1, 2) << std::endl
		<< "2-1=" << Sub(2, 1) << std::endl;

	system("pause");
	return 0;
}