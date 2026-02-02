#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include "EventLoop.h"

class WorkerThread {
public:
	WorkerThread(int index);
	~WorkerThread();
	//启动子线程
	void run();
	//获取子线程的反应堆实例
	inline EventLoop* getEventLoop() {
		return m_evLoop;
	}

private:
	//回调函数
	void running();

private:
	std::thread* m_thread;
	std::thread::id m_threadID;
	std::string m_name;
	std::mutex m_mutex;
	std::condition_variable m_cond;
	EventLoop* m_evLoop;//子线程的反应堆
};