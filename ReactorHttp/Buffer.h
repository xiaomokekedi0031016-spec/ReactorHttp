#pragma once
#include <string>

class Buffer {
public:
	Buffer(int size);
	~Buffer();

	//得到剩余可写的内存容量
	inline int writeableSize() {
		return m_capacity - m_writePos;
	}	

	//得到剩余可读的内存容量(已写)
	inline int readableSize() {
		return m_writePos - m_readPos;
	}

	//内存扩容
	void extendRoom(int size);

	//写内存
	int appendString(const char* data, int size);
	int appendString(const char* data);
	int appendString(const std::string data);
	
	//读内存
	int socketRead(int fd);	

	//发送数据
	int sendData(int socket);

	//根据\r\n取出一行,找到其在数据块中的位置,返回该位置
	char* findCRLF();	
	//得到读数据的起始位置
	inline char* data() {
		return m_data + m_readPos;
	}	
	inline int readPosIncrease(int count) {
		m_readPos += count;
		return m_readPos;
	}	

private:
	char* m_data;//指向内存的指针
	int m_capacity;//容量
	int m_readPos = 0;//读位置
	int m_writePos = 0;//写位置
};