#pragma once
#include <functional>

enum class FDEvent {
	TimeOut = 0x01,
	ReadEvent = 0x02,
	WriteEvent = 0x04
};

class Channel {
public:
	using handleFunc = std::function<int(void*)>;

	handleFunc readCallback;
	handleFunc writeCallback;
	handleFunc destroyCallback;//销毁回调

	// 修改fd的写事件(检测 or 不检测)
	void writeEventEnable(bool flag);
	// 判断是否需要检测文件描述符的写事件
	bool isWriteEventEnable();

public:
	Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* arg);

	inline int getSocket() {
		return m_fd;
	}

	inline int getEvent() {
		return m_events;
	}

	inline const void* getArg() {
		return m_arg;
	}


private:
	int m_fd;
	int m_events;
	void* m_arg;//注册Channel的回调函数的参数

};