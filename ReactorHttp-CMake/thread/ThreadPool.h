#pragma once
#include "EventLoop.h"
#include <vector>
#include "WorkerThread.h"		
#include <stdbool.h>

class ThreadPool
{
public:
	ThreadPool(EventLoop* mainLoop, int count);
	~ThreadPool();
	//启动线程池
	void run();
	//取出线程池中的某个子线程的反应堆实例
	EventLoop* takeWorkerEventLoop();

private:
	EventLoop* m_mainLoop;//主反应堆
	bool m_isStart;//线程池是否启动
	int m_threadNum;//线程数量
	std::vector<WorkerThread*> m_workerThreads;//工作线程数组
	int m_index;//轮询索引 >> 取子线程反应堆
};