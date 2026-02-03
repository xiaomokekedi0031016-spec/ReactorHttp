#pragma once
#include "EventLoop.h"
#include <string>
#include "Channel.h"
#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class TcpConnection {
public:
	TcpConnection(int fd, EventLoop* evloop);
	~TcpConnection();

	static int processRead(void* arg);
	static int processWrite(void* arg);
	static int destroy(void* arg);

private:
	EventLoop* m_evLoop;//子线程的反应堆
	std::string m_name;//连接名称
	Channel* m_channel;//通道	
	Buffer* m_readBuf;//读缓冲区	
	Buffer* m_writeBuf;//写缓冲区
	HttpRequest* m_request;
	HttpResponse* m_response;
};