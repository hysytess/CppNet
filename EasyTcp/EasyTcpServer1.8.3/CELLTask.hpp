#ifndef _CELL_TASK_H_

#include <thread>
#include <mutex>
#include <list>

// 任务虚基类 提供接口
class CellTask
{
public:
	CellTask() {}
	virtual ~CellTask() {}
	//执行任务
	virtual void doTask() {}
private:
};

// 任务类[实现]
class CellTaskServer
{
private:
	// 任务队列
	std::list<CellTask*> _tasks;
	// 任务队列缓冲区
	std::list<CellTask*> _tasksBuff;
	std::mutex _mutex;
public:
	CellTaskServer()
	{
	}
	~CellTaskServer()
	{
	}
	//添加任务到缓冲区队列
	void addTask(CellTask* task)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuff.push_back(task);
	}
	// 启动服务
	void Start()
	{
		std::thread t(std::mem_fun(&CellTaskServer::OnRun), this);
		t.detach();
	}
	// 工作函数 从缓冲区取出任务事件放到任务队列中并顺序执行队列中的任务
protected:
	void OnRun()
	{
		while (true)
		{
			if (!_tasksBuff.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuff)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuff.clear();
			}
			if (_tasksBuff.empty())
			{
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			// 处理任务
			for (auto pTask : _tasks)
			{
				pTask->doTask();
				delete pTask;
			}
			// 清空任务队列
			_tasks.clear();
		}
	}
};
#endif