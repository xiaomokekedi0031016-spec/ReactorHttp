#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include <string>

class EventLoop;
class Dispatcher {
public:
	Dispatcher(EventLoop* evloop);
	virtual ~Dispatcher();
	virtual int add();
	virtual int remove();
	virtual int modify();
	virtual int dispatch(int timeout = 2); // 单位: s
	inline void setChannel(Channel* channel)
	{
		m_channel = channel;
	}


protected:
	EventLoop* m_evLoop;//反应堆
	Channel* m_channel;//通道
	std::string m_name = std::string();
};