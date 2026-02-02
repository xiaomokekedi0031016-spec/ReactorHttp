#include "WorkerThread.h"
#include <string>

WorkerThread::WorkerThread(int index) {
	m_evLoop = nullptr;
	m_thread = nullptr;
	m_threadID = std::thread::id();
	m_name = "SubThread-" + std::to_string(index);
}

WorkerThread::~WorkerThread() {
	if (m_thread != nullptr) {
		delete m_thread;
	}
}

void WorkerThread::running() {
	m_mutex.lock();
	m_evLoop = new EventLoop(m_name);
	m_mutex.unlock();
	m_cond.notify_one();
	m_evLoop->run();
}

void WorkerThread::run() {
	//创建子线程,回调在过这一行就会执行
	m_thread = new std::thread(&WorkerThread::running, this);
	//阻塞主线程,让当前函数不会直接结束
	//std::unique_lock配合条件变量使用,因为可以临时解锁
	std::unique_lock<std::mutex> locker(m_mutex);
	while (m_evLoop == nullptr) {
		m_cond.wait(locker);
	}
}