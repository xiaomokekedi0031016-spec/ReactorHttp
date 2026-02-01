#include "Channel.h"

Channel::Channel(int fd,
	FDEvent events,
	handleFunc readFunc,
	handleFunc writeFunc,
	handleFunc destroyFunc,
	void* arg) 
	: m_fd(fd)
	, m_arg(arg)
	, m_events(static_cast<int>(events))
	, readCallback(readFunc)
	, writeCallback(writeFunc)
	, destroyCallback(destroyFunc)
{}