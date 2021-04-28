#ifndef _CELLSEMAPHORE_HPP_
#define _CELLSEMAPHORE_HPP_

#include <chrono>
#include <thread>
// 信号量 简单实现
class  CELLSemaphore
{
public:
	CELLSemaphore(){}
	~CELLSemaphore(){}
	void wait()
	{
		_isWaitExit = true;
		while (_isWaitExit)
		{
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
		}
	}

	void wakeup()
	{
		_isWaitExit = false;
	}
private:
	bool _isWaitExit;
};

#endif
