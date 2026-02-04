#pragma once
#include <string>
#include <functional>
#include <map>
#include "Buffer.h"

//状态码
enum class StatusCode {
	Unknown,//未知
	OK = 200,//成功
	MovedPermanently = 301,//永久重定向
	MovedTemporarily = 302,//临时重定向
	BadRequest = 400,//错误请求
	NotFound = 404,//未找到
	NoContent = 204
};;

class HttpResponse {
public:
	//注册回调
	std::function<void(const std::string, struct Buffer*, int)> sendDataFunc;
	HttpResponse();
	~HttpResponse();	
	//设置回复的文件名
	inline void setFileName(std::string name){
		m_fileName = name;
	}
	//设置状态码
	inline void setStatusCode(StatusCode code){
		m_statusCode = code;
	}

	//添加响应头
	void addHeader(const std::string key, const std::string value);	
	// 组织http响应数据
	void prepareMsg(Buffer* sendBuf, int socket);

private:
	//状态码
	StatusCode m_statusCode;
	//文件名
	std::string m_fileName;
	//响应头-键值对
	std::map<std::string, std::string> m_headers;	
	// 定义状态码和描述的对应关系
	const std::map<int, std::string> m_info = {
		{200, "OK"},
		{301, "MovedPermanently"},
		{302, "MovedTemporarily"},
		{400, "BadRequest"},
		{404, "NotFound"},
	};
};