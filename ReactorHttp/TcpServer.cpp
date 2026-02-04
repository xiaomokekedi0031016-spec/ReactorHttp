#include "TcpServer.h"
#include "Log.h"
#include <arpa/inet.h>
#include "TcpConnection.h"

TcpServer::TcpServer(unsigned short port, int threadNum) {
	//Debug("TcpServer::TcpServer()....");
	m_port = port;
	m_mainLoop = new EventLoop;
	m_threadNum = threadNum;
	m_threadPool = new ThreadPool(m_mainLoop, threadNum);
	setListen();
}

//初始化监听
void TcpServer::setListen() {
	m_lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_lfd == -1)
	{
		perror("socket");
		return;
	}
	int opt = 1;
	int ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
	if(ret == -1) {
		perror("setsockopt");
		return;
	}
	struct sockaddr_in addr;//ipv4专用地址结构体
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);//转化为网络字节序
	addr.sin_addr.s_addr = INADDR_ANY;
	ret = bind(m_lfd, (struct sockaddr*)&addr, sizeof addr);
	if (ret == -1)
	{
		perror("bind");
		return;
	}
	ret = listen(m_lfd, 128);
	if (ret == -1)
	{
		perror("listen");
		return;
	}
}

int TcpServer::acceptConnection(void* arg) {
	//Debug("11111111111111111");
	TcpServer* server = static_cast<TcpServer*>(arg);
	// 和客户端建立连接
	int cfd = accept(server->m_lfd, NULL, NULL);
	// 从线程池中取出一个子线程的反应堆实例, 去处理这个cfd
	EventLoop* evLoop = server->m_threadPool->takeWorkerEventLoop();
	// 将cfd放到 TcpConnection中处理
	new TcpConnection(cfd, evLoop);
	return 0;
}

//启动服务器
void TcpServer::run() {
	//todo..
	Debug("服务器程序已经启动了...");
	//启动线程池
	m_threadPool->run();
	Channel* channel = new Channel(m_lfd, FDEvent::ReadEvent, acceptConnection, nullptr, nullptr, this);
	m_mainLoop->addTask(channel, ElemType::ADD);
	// 启动主线程的反应堆模型
	m_mainLoop->run();

}

