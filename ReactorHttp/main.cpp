#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//argc表示命令行参数的个数
//argv表示命令行参数的数组
int main(int argc, char* argv[])
{
#if 1
	if (argc < 3) {
		printf("./a.out port path\n");
		return -1;
	}
	unsigned short port = atoi(argv[1]);
	//切换服务器的工作路径
	chdir(argv[2]);

#else
	usigned short port = 10000;
	chdir("/home/wz/luffy");
#endif
	//启动服务器

	return 0;
}