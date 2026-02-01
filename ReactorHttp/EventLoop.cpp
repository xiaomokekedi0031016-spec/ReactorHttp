#include "EventLoop.h"
#include <string>
#include "EpollDispatcher.h"
#include <sys/socket.h>
#include <cassert>

EventLoop::EventLoop()
	:EventLoop(std::string()){}

EventLoop::EventLoop(const std::string threadName) {
	m_isQuit = true;//默认没有启动，启动即循环进行事件处理
	m_threadID = std::this_thread::get_id();
	m_threadName = threadName == std::string() ? "MainThread" : threadName;
	m_dispatcher = new EpollDispatcher(this);
	m_channelMap.clear();
	//创建本地套接字
	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_socketPair);
	if (ret == -1) {
		perror("socketpair");
		exit(0);
	}

}

EventLoop::~EventLoop() {

}

int EventLoop::run() {
	m_isQuit = false;
	if(m_threadID != std::this_thread::get_id()) {
		return -1;
	}
	while (!m_isQuit) {
		m_dispatcher->dispatch();//超时时长2s
		//...todo
	}

	return 0;
}

int EventLoop::eventActive(int fd, int event) {
	if(fd < 0) {
		return -1;
	}
	Channel* channel = m_channelMap[fd];
	assert(channel->getSocket() == fd);
	if (event & (int)FDEvent::ReadEvent && channel->readCallback) {
		channel->readCallback(const_cast<void*>(channel->getArg()));
	}
	if (event & (int)FDEvent::WriteEvent && channel->writeCallback) {
		channel->writeCallback(const_cast<void*>(channel->getArg()));
	}

	return 0;
}

int EventLoop::addTask(Channel* channel, ElemType type) {
	m_mutex.lock();
	ChannelElement* node = new ChannelElement();
	node->channel = channel;
	node->type = type;
	m_taskQ.push(node);
	m_mutex.unlock();
	//处理节点
	if(m_threadID == std::this_thread::get_id()) {
		processTaskQ();
	} else {
		taskWakeup();
	}
}