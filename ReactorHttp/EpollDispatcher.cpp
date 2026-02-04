#include "EpollDispatcher.h"
#include <sys/epoll.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

EpollDispatcher::EpollDispatcher(EventLoop* evloop) 
	:Dispatcher(evloop)
{
	m_epfd = epoll_create(10);// 参数随意, 只要大于0
	if (m_epfd == -1) {
		perror("epoll_create");
		exit(0);
	}

	m_events = new struct epoll_event[m_maxNode];

	m_name = "Epoll";
}

EpollDispatcher::~EpollDispatcher() {
	close(m_epfd);
}

int EpollDispatcher::add() {
	int ret = epollCtl(EPOLL_CTL_ADD);
	if (ret == -1) {
		perror("epoll_ctl add");
		exit(0);
	}
	return ret;
}

int EpollDispatcher::remove() {
	int ret = epollCtl(EPOLL_CTL_DEL);
	if (ret == -1) {
		perror("epoll_ctl delete");
		exit(0);
	}
	//todo...

	return ret;
}

int EpollDispatcher::modify() {
	int ret = epollCtl(EPOLL_CTL_MOD);
	if (ret == -1) {
		perror("epoll_ctl modify");
		exit(0);
	}
	return ret;
}

int EpollDispatcher::dispatch(int timeout)// 单位: s
{
	int count = epoll_wait(m_epfd, m_events, m_maxNode, timeout * 1000);
	for (int i = 0; i < count; ++i) {
		int events = m_events[i].events;
		int fd = m_events[i].data.fd;
		if (events & EPOLLERR || events & EPOLLHUP) {
			continue;
		}
		if(events & EPOLLIN) {
			//...todo
			m_evLoop->eventActive(fd, (int)FDEvent::ReadEvent);
		}
		if (events & EPOLLOUT) {
			//...todo
			m_evLoop->eventActive(fd, (int)FDEvent::WriteEvent);
		}
	}
	return 0;
}

int EpollDispatcher::epollCtl(int op) {
	//epoll_event结构体变量，用于注册事件到epoll实例中
	struct epoll_event ev;
	ev.data.fd = m_channel->getSocket();
	int events = 0;
	if(m_channel->getEvent() & (int)FDEvent::ReadEvent) {//与
		events |= EPOLLIN;
	}
	if(m_channel->getEvent() & (int)FDEvent::WriteEvent) {
		events |= EPOLLOUT;
	}
	ev.events = events;
	//将需要监听的文件描述符添加到epoll实例中
	int ret = epoll_ctl(m_epfd, op, m_channel->getSocket(), &ev);
	
	return ret;
}