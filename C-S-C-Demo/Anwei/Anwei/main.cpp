// 加入多线程控制台
// 兼容多平台

#include <iostream>
#include <algorithm>
#include <thread>
#include <string>
#include <set>
#include <map>
#include <cstdlib>


// 第一题 求补集
void test1()
{
	std::string setA;
	std::string setB;
	std::cin >> setA;
	std::cin >> setB;

	if (!(setA.length() * setB.length())) return;

	std::set<char> setSrc;
	if (setA.length() > setB.length())
	{
		for (auto i : setB)
			setSrc.insert(i);

		for (auto l : setA)
			if (!setSrc.count(l))
				std::cout << l << " ";
		std::cout << std::endl;
	}
	else
	{
		for (auto i : setA)
			setSrc.insert(i);

		for (auto l : setB)
			if (!setSrc.count(l))
				std::cout << l << " ";
		std::cout << std::endl;
	}
}

// 将数组中的0移至数组末尾
void test2()
{
	std::string src, ans;
	std::cin >> src;
	int cnt = 0;
	ans += "[";

	for (size_t i = 1; i < src.length() - 1; i++)
	{
		if (src[i] != '0' && src[i] != ',')
		{
			int j = i;
			while (src[j] != ','&&src[j] != ']')
			{
				ans += src[j];
				j++;
			}
			i = j;
			ans += ",";
		}
		else if (src[i] == '0')
		{
			cnt++;
		}
	}

	for (int i = 0; i < cnt; i++)
		ans += "0,";

	ans[ans.length() - 1] = ']';
	std::cout << ans << std::endl;
}

size_t Mystrlen(const char* src)
{
	return *(src++) == '\0' ? 0 : Mystrlen(src++) + 1;
}


int main(int argc, char* args[])
{
	system("pause");
	return 0;
}