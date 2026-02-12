#include "HttpRequest.h"
#include <functional>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "Log.h"

void HttpRequest::reset() {
	m_curState = PrecessState::ParseReqLine;
	m_method = m_url = m_version = std::string(); 
	m_reqHeaders.clear();
}

HttpRequest::HttpRequest()
{
	reset();
}

HttpRequest::~HttpRequest(){
}	

void HttpRequest::addHeader(const std::string key, const std::string value) {
	if (key.empty() || value.empty()){
		return;
	}
	m_reqHeaders.insert(make_pair(key, value));
}

char* HttpRequest::splitRequestLine(const char* start, const char* end, const char* sub, std::function<void(std::string)> callback)
{
	char* space = const_cast<char*>(end);
	if (sub != nullptr)
	{
		space = static_cast<char*>(memmem(start, end - start, sub, strlen(sub)));
		assert(space != nullptr);
	}
	int length = space - start;
	callback(std::string(start, length));
	return space + 1;
}

int HttpRequest::hexToDec(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0;
}

bool HttpRequest::parseRequestLine(Buffer* readBuf) {
	char* end = readBuf->findCRLF();
	char* start = readBuf->data();
	int lineSize = end - start;//请求行的长度
	if (lineSize > 0) {
		auto methodFunc = std::bind(&HttpRequest::setMethod, this, std::placeholders::_1);
		start = splitRequestLine(start, end, " ", methodFunc);
		auto urlFunc = std::bind(&HttpRequest::seturl, this, std::placeholders::_1);
		start = splitRequestLine(start, end, " ", urlFunc);
		auto versionFunc = std::bind(&HttpRequest::setVersion, this, std::placeholders::_1);
		splitRequestLine(start, end, nullptr, versionFunc);
		//为解析请求头做准备
		readBuf->readPosIncrease(lineSize + 2);
		//修改状态
		setState(PrecessState::ParseReqHeaders);
		return true;
	}

	return false;
}

std::string HttpRequest::decodeMsg(std::string msg) {
	std::string str = std::string();
	const char* from = msg.data();
	for (; *from != '\0'; ++from) {
		//处理中文,特殊字符需要这样 >> 然后hexToDec把16进制转换成10进制 >> 得到原始字符
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
			str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));
			from += 2;
		}
		else{
			str.append(1, *from);
		}
	}
	str.append(1, '\0');
	return str;
}

bool HttpRequest::parseRequestHeader(Buffer* readBuf)
{
	char* end = readBuf->findCRLF();
	if (end != nullptr) {
		char* start = readBuf->data();
		int lineSize = end - start;
		char* middle = static_cast<char*>(memmem(start, lineSize, ": ", 2));
		if (middle != nullptr) {
			int keyLen = middle - start;
			int valueLen = end - middle - 2;
			if(keyLen > 0 && valueLen >0) {
				std::string key(start, keyLen);
				std::string value(middle + 2, valueLen);
				addHeader(key, value);
			}
			readBuf->readPosIncrease(lineSize + 2);	
		}
		else {
			//请求头被解析完了
			readBuf->readPosIncrease(2);//跳过空行
			setState(PrecessState::ParseReqDone);
		}
		return true;
	}
	return false;
}

const std::string HttpRequest::getFileType(const std::string name)
{
	// a.jpg a.mp4 a.html
	// 自右向左查找‘.’字符, 如不存在返回NULL
	const char* dot = strrchr(name.data(), '.');
	if (dot == NULL)
		return "text/plain; charset=utf-8";	// 纯文本
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";
}

bool HttpRequest::processHttpRequest(HttpResponse* response) {
	if (strcasecmp(m_method.data(), "get") != 0) {
		return -1;
	}
	m_url = decodeMsg(m_url);
	if (m_url == "/favicon.ico") {
		// 直接返回 204 No Content，不走 sendFile
		response->setFileName("");  // 不需要文件
		response->setStatusCode(StatusCode::NoContent); // 204
		response->addHeader("Connection", "keep-alive");
		response->sendDataFunc = nullptr; // 没有数据发送
		Debug("拦截 /favicon.ico 请求，返回 204 响应");
		return true; // 已处理
	}
	//处理客户端请求的静态资源(目录或者文件)
	const char* file = NULL;
	if(strcmp(m_url.data(), "/") == 0){
		file = "./";//资源根目录
	}
	else {
		file = m_url.data() + 1;
	}
	//获取文件属性
	struct stat st;
	int ret = stat(file, &st);
	if (ret == -1) {
		response->setFileName("404.html");
		response->setStatusCode(StatusCode::NotFound);
		response->addHeader("Content-type", getFileType(".html"));
		response->sendDataFunc = sendFile;
		return 0;
	}

	response->setFileName(file);
	response->setStatusCode(StatusCode::OK);
	if (S_ISDIR(st.st_mode)) {
		response->addHeader("Content-type", getFileType(".html"));
		response->sendDataFunc = sendDir;
	}
	else {
		response->addHeader("Content-type", getFileType(file));
		response->addHeader("Content-length", std::to_string(st.st_size));
		response->sendDataFunc = sendFile;
	}
	return false;
}


bool HttpRequest::parseHttpRequest(Buffer* readBuf, HttpResponse* response, Buffer* sendBuf, int socket) {
	bool flag = true;
	while (m_curState != PrecessState::ParseReqDone) {
		switch (m_curState)
		{
		case PrecessState::ParseReqLine:
			flag = parseRequestLine(readBuf);
			break;
		case PrecessState::ParseReqHeaders:
			flag = parseRequestHeader(readBuf);
			break;
		case PrecessState::ParseReqBody://忽略post请求
			break;
		default:
			break;
		}
		if (!flag) {
			return flag;
		}
		//判断是否解析完毕了, 如果完毕了, 需要准备回复的数据
		if (m_curState == PrecessState::ParseReqDone) {
			processHttpRequest(response);
			//发生给客户端
			response->prepareMsg(sendBuf, socket);
		}
	}
	m_curState = PrecessState::ParseReqLine;   // 状态还原, 保证还能继续处理第二条及以后的请求
	return flag;
}

void HttpRequest::sendFile(std::string fileName, Buffer* sendBuf, int cfd) {
	Debug("sendFile: cfd=%d, file=%s", cfd, fileName.c_str());
	int fd = open(fileName.data(), O_RDONLY);
	if (fd < 0) {//修改
		perror("open");
		return;
	}
	while (1) {
		char buf[1024];
		int len = read(fd, buf, sizeof buf);
		if (len > 0) {
			sendBuf->appendString(buf, len);
#ifndef MSG_SEND_AUTO
			sendBuf->sendData(cfd);
#endif
		}
		else if (len == 0){
			break;
		}
		else{
			perror("read");
			break;//修改
		}
	}
	close(fd);	
}

void HttpRequest::sendDir(std::string dirName, Buffer* sendBuf, int cfd) {
	char buf[4096] = { 0 };
	sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName.data());
	struct dirent** namelist;
	int num = scandir(dirName.data(), &namelist, NULL, alphasort);
	for (int i = 0; i < num; ++i)
	{
		// 取出文件名 namelist 指向的是一个指针数组 struct dirent* tmp[]
		char* name = namelist[i]->d_name;
		struct stat st;
		char subPath[1024] = { 0 };
		sprintf(subPath, "%s/%s", dirName.data(), name);
		stat(subPath, &st);
		if (S_ISDIR(st.st_mode))
		{
			// a标签 <a href="">name</a>
			sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
				name, name, st.st_size);
		}
		else
		{
			sprintf(buf + strlen(buf),
				"<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
				name, name, st.st_size);
		}
		// send(cfd, buf, strlen(buf), 0);
		sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
		sendBuf->sendData(cfd);
#endif
		memset(buf, 0, sizeof(buf));
		free(namelist[i]);
	}
	sprintf(buf, "</table></body></html>");
	// send(cfd, buf, strlen(buf), 0);
	sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
	sendBuf->sendData(cfd);
#endif 

	free(namelist);
}










