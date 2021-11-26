#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>




#define READ_FIFO "/tmp/read_fifo"
#define WRITE_FIFO "/tmp/write_fifo"
//本程序从一个FIFO读数据，并把读到的数据打印到标准输出
//如果读到字符“Q”，则退出
int main(int argc, char** argv)
{
	int ret;
	char buf_r[100];
	int fd;
	int nread;
	int nwrite;
	fd_set read_fdset,write_fdset;
	struct timeval respond_timeout;
	if((mkfifo(READ_FIFO, O_CREAT) < 0) && (errno != EEXIST))
	{
	        printf("不能创建FIFO\n");
	        exit(1);
	}

	if((mkfifo(WRITE_FIFO, O_CREAT) < 0) && (errno != EEXIST))
	{
	        printf("不能创建FIFO\n");
	        exit(1);
	}

	printf("准备读取数据\n");
	nread = open(READ_FIFO, O_RDONLY, 0);
	if(fd == -1)
	{
	        perror("打开FIFO");
	        exit(1);
	}

	nwrite = open(WRITE_FIFO, O_WRONLY, 0);
	if(fd == -1)
	{
	        perror("打开FIFO");
	        exit(1);
	}

	//while(1)
	//{
	 	FD_ZERO(&read_fdset);
		FD_SET(nread,&read_fdset);

		write(nwrite,"touch /tmp/lin",strlen("touch /tmp/lin"));

		respond_timeout.tv_sec=2;
		respond_timeout.tv_usec=0;

		ret=select(nread+1, &read_fdset, NULL, NULL,&respond_timeout);
		if(ret==-1)
			printf("timeout\n");
		else
			ret=read(nread,buf_r,sizeof(buf_r));
			printf("buf_r %s\n",buf_r);
	//}

	close(nread);
	close(nwrite);

}
