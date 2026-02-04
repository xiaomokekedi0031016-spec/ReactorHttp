#pragma once
#include <thread>
#include <map>
#include "Dispatcher.h"
#include <mutex>
#include <queue>
#include "Channel.h"

//处理节点的方式
enum class ElemType : char { ADD, DELETE, MODIFY };

//任务队列的节点
struct ChannelElement {
	ElemType type;//处理节点的方式
	Channel* channel;
};

class Dispatcher;
class EventLoop {
public:
	EventLoop();
	EventLoop(const std::string threadName);
	~EventLoop();
	//启动反应堆模型
	int run();
	//处理被激活的文件fd
	int eventActive(int fd, int event);
	//添加任务到任务队列中
	int addTask(Channel* channel, ElemType type);
	//处理任务队列中的任务
	int processTaskQ();
	//处理dispatcher中的节点
	int add(Channel* channel);
	int modify(Channel* channel);
	int remove(Channel* channel);
	//唤醒子线程后,子线程反应堆执行的本地套接字通信
	int readMessage();
	//释放channel
	int freeChannel(Channel* channel);	
	inline std::thread::id getThreadID() {
		return m_threadID;
	}
	//inline std::string getThreadName()
	//{
	//	return m_threadName;
	//}

	////静态成员函数是不依赖对象,配合this回传使用
	//static int readLocalMessage(void* arg);

private:
	//唤醒子线程
	void taskWakeup();

private:
	bool m_isQuit;//反应堆开关
	std::thread::id m_threadID;//线程ID
	std::string m_threadName;//当前线程名称(区分主线程和工作线程)
	Dispatcher* m_dispatcher;//派发器指针
	std::map<int, Channel*> m_channelMap;//通道映射表
	std::mutex m_mutex;
	int m_socketPair[2];//本地通信socketpair
	std::queue<ChannelElement*> m_taskQ;
};