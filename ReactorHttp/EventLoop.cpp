#include "EventLoop.h"
#include <string>
#include "EpollDispatcher.h"
#include <sys/socket.h>
#include <cassert>
#include <unistd.h>
#include <string.h>

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
	
	auto obj = std::bind(&EventLoop::readMessage, this);
	Channel* channel = new Channel(
		m_socketPair[1],
		FDEvent::ReadEvent,
		obj,
		nullptr,
		nullptr,
		this
	);	

	addTask(channel, ElemType::ADD);
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
		processTaskQ();
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
		//如果是子线程处理任务队列
		//这里也可能是主线程发起的添加监听lfd
		processTaskQ();
	} else {
		//这里一定是主线程 >> 第一次new connection的时候会走到这里, 所有不会出现通信自己的本地套接字情况
		taskWakeup();
	}
	return 0;
}

int EventLoop::processTaskQ() {
	while (!m_taskQ.empty()) {
		m_mutex.lock();
		ChannelElement* node = m_taskQ.front();
		m_taskQ.pop();
		m_mutex.unlock();
		Channel* channel = node->channel;
		if (node->type == ElemType::ADD) {
			add(channel);
		}
		else if (node->type == ElemType::DELETE) {
			remove(channel);
		}
		else if (node->type == ElemType::MODIFY) {
			modify(channel);
		}
		delete node;
	}
	return 0;
}

int EventLoop::add(Channel* channel) {
	int fd = channel->getSocket();
	if(m_channelMap.find(fd) == m_channelMap.end()) {
		m_channelMap.insert(std::make_pair(fd, channel));
		//todo...
		m_dispatcher->setChannel(channel);
		int ret = m_dispatcher->add();
		return ret;
	}
	return -1;
}

int EventLoop::modify(Channel* channel) {
	int fd = channel->getSocket();
	if (m_channelMap.find(fd) == m_channelMap.end()) {
		return -1;
	}
	//todo...
	m_dispatcher->setChannel(channel);
	int ret = m_dispatcher->modify();
	return ret;
}

int EventLoop::remove(Channel* channel) {
	int fd = channel->getSocket();
	if(m_channelMap.find(fd) == m_channelMap.end()) {
		return -1;
	}
	//todo...
	m_dispatcher->setChannel(channel);
	int ret = m_dispatcher->remove();	
	return ret;
}

void EventLoop::taskWakeup() {
	const char* msg = "hello world!!!";
	write(m_socketPair[0], msg, strlen(msg));
}

int EventLoop::readMessage() {
	char buf[256];
	read(m_socketPair[1], buf, sizeof(buf));
	return 0;
}

int EventLoop::freeChannel(Channel* channel) {
	auto it = m_channelMap.find(channel->getSocket());
	if(it != m_channelMap.end()) {
		m_channelMap.erase(it);
		close(channel->getSocket());
		delete channel;
	}	
	return 0;
}


