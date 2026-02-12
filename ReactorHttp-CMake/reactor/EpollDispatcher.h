#pragma once
#include "Dispatcher.h"

class EpollDispatcher : public Dispatcher
{
public:
	EpollDispatcher(EventLoop* evloop);
	~EpollDispatcher();

	int add() override;
	int remove() override;
	int modify() override;
	int dispatch(int timeout = 2) override; // 单位: s

private:
	int epollCtl(int op);

private:
	int m_epfd;//红黑树句柄
	struct epoll_event* m_events;//内核返回就绪事件数组
	const int m_maxNode = 520;//最大监听节点数
};