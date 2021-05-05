#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include "CELLSemaphore.hpp"

class CellThread
{
public:
	static void Sleep(time_t dt)
	{
		std::chrono::milliseconds t(dt);
		std::this_thread::sleep_for(t);
	}
private:
	typedef std::function<void(CellThread*)> EventCall;
public:
	void Start(
		EventCall onCreate = nullptr,
		EventCall onRun = nullptr,
		EventCall OnDestory = nullptr)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (!_isRun)
		{
			_isRun = true;

			if (onCreate)
				_OnCreate = onCreate;
			if (onRun)
				_OnRun = onRun;
			if (OnDestory)
				_OnDestory = OnDestory;

			std::thread t(std::mem_fun(&CellThread::OnWork), this);
			t.detach();
		}

	}

	void Close()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
			_sem.wait();
		}
	}

	void Exit()
	{
		std::lock_guard<std::mutex> lock(_mutex);
		if (_isRun)
		{
			_isRun = false;
		}
	}
	
	bool isRun()
	{
		return _isRun;
	}
protected:
	void OnWork()
	{
		if (_OnCreate)
			_OnCreate(this);
		if (_OnRun)
			_OnRun(this);
		if (_OnDestory)
			_OnDestory(this);
		_sem.wakeup();
		_isRun = false;
	}
private:
	EventCall _OnCreate;
	EventCall _OnRun;
	EventCall _OnDestory;

	std::mutex _mutex;
	CELLSemaphore _sem;
	//线程是否运行
	bool _isRun = false;

};

#endif
