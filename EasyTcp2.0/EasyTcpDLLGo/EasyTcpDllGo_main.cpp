
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
}


