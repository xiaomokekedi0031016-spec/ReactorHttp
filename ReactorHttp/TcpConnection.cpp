#include "TcpConnection.h"

int TcpConnection::processRead(void* arg) {
	TcpConnection* conn = static_cast<TcpConnection*>(arg);
}

int TcpConnection::processWrite(void* arg) {

}

int TcpConnection::destroy(void* arg) {

}

TcpConnection::TcpConnection(int fd, EventLoop* evloop) {
	m_evLoop = evloop;
	m_name = "Connection-" + std::to_string(fd);
	//todo...
	m_readBuf = new Buffer(10240);
	m_writeBuf = new Buffer(10240);
	m_request = new HttpRequest;
	m_response = new HttpResponse;
	//主线程调用子线程对象的函数，代码依然在主线程执行，只是操作了子线程的数据。正因为这是“跨线程操作数据”，所以才需要锁和唤醒机制。
    //主线程调用的，但是用的是子线程的反应堆
	m_channel = new Channel(fd, FDEvent::ReadEvent, processRead, processWrite, destroy, this);
	evloop->addTask(m_channel, ElemType::ADD);
}

TcpConnection::~TcpConnection() {
	if (m_readBuf && m_readBuf->readableSize() == 0 &&
		m_writeBuf && m_writeBuf->readableSize() == 0)
	{
		delete m_readBuf;
		delete m_writeBuf;
		delete m_request;
		delete m_response;
		m_evLoop->freeChannel(m_channel);
	}
	//todo...

}


