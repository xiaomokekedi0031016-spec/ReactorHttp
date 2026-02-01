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