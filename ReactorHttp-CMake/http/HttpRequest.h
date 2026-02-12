#pragma once
#include "Buffer.h"
#include <string>
#include <map>
#include "HttpResponse.h"
#include <functional>

//当前的解析状态
enum class PrecessState:char
{
	ParseReqLine,//解析请求行
	ParseReqHeaders,//解析请求头
	ParseReqBody,//解析请求体
	ParseReqDone//解析完成
};

class HttpRequest {
public:
	HttpRequest();
	~HttpRequest();
	// 重置
	void reset();
	// 添加请求头
	void addHeader(const std::string key, const std::string value);	
	// 解析请求行
	bool parseRequestLine(Buffer* readBuf);
	// 解析请求头
	bool parseRequestHeader(Buffer* readBuf);
	//设置请求方法"Get"
	inline void setMethod(std::string method) {
		m_method = method;
	}
	//设置请求url"/index.html"
	inline void seturl(std::string url) {
		m_url = url;
	}
	//设置请求协议版本"Http/1.1"
	inline void setVersion(std::string version) {
		m_version = version;
	}
	//设置处理状态
	inline void setState(PrecessState state) {
		m_curState = state;
	}
	//解析http请求协议
	bool parseHttpRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket);
	// 处理http请求协议
	bool processHttpRequest(HttpResponse* response);
	//解码字符串
	std::string decodeMsg(std::string from);
	//获取文件类型
	const std::string getFileType(const std::string name);
	//发送目录或文件的静态函数
	static void sendDir(std::string fileName, Buffer* sendBuf, int cfd);
	static void sendFile(std::string fileName, Buffer* sendBuf, int cfd);

private:
	//处理请求行的每一个消息段
	char* splitRequestLine(const char* start,const char* end, const char* delim, std::function<void(std::string)> callback);
	//16进制转10进制
	int hexToDec(char c);

private:
	std::string m_method;//"Get"
	std::string m_url;//"/index.html"
	std::string m_version;//"Http/1.1"
	std::map<std::string, std::string> m_reqHeaders;//保存请求头
	PrecessState m_curState;//当前解析状态
};