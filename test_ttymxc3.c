#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
 
int uart_init(int fd)
{
	struct termios newtio, oldtio;
	fcntl(fd,F_SETFL,0);/*恢复串口为阻塞状态*/
    if ( tcgetattr(fd, &oldtio ) != 0) {  
        perror("tcgetattr error");
        return -1;
    }
	bzero( &newtio, sizeof(newtio) );
    newtio.c_cflag  |= B115200 | CLOCAL | CREAD; // 设置波特率，本地连接，接收使能
    newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag  |= CS8; // 数据位为 8 ，CS7 for 7 
	newtio.c_cflag &= ~CSTOPB; // 一位停止位， 两位停止为 |= CSTOPB
	newtio.c_cflag &= ~PARENB; // 无校验
	//newtio.c_cflag |= PARENB; //有校验
	//newtio.c_cflag &= ~PARODD // 偶校验
	//newtio.c_cflag |=  PARODD    // 奇校验
	//newtio.c_cc[VTIME] = 0; // 等待时间，单位百毫秒 （读）。后有详细说明
	//newtio.c_cc[VMIN] = 0; // 最小字节数 （读）。后有详细说明
	tcflush(fd, TCIOFLUSH); // TCIFLUSH刷清输入队列。
                                       //TCOFLUSH刷清输出队列。 
                                       //TCIOFLUSH刷清输入、输出队列。
	tcsetattr(fd, TCSANOW, &newtio); // TCSANOW立即生效；
                                                        //TCSADRAIN：Wait until everything has been transmitted；
                                                        //TCSAFLUSH：Flush input and output buffers and make the change	
	
	return 0;
}
 
int main(int argc, char *argv[])
{
	char buff[50] = {0};
	//char w_buff[] = {0xaa, 0x55, 0x00, 0x30, 0x00, 0x08, 0x00, 0x00, 0xff, 0x07, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00, 0x00, 0x0d, 0x0a };
	char w_buff[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa};
	int read_size = 0, i = 0, write_size = 0;
	//int fd = open("/dev/ttyAMA0", O_RDWR|O_NOCTTY|O_NDELAY);
	int fd = open("/dev/ttymxc3", O_RDWR|O_NOCTTY);
	if(-1 == fd){
		perror("can't open serialport");
		return -1;
	}
	
	uart_init(fd);
 
	while(1)
	{
		write_size = write(fd, w_buff, sizeof(w_buff));
		if(write_size < 0){
			perror("serial port write is error");
		}
		
		read_size = read(fd, buff, sizeof(buff));
		printf("write_size = %d .receive length:%d data:", write_size, read_size);
		for(i = 0; i < read_size; i++){
			printf("%02x ", buff[i]);
		}
		printf("\n");
		usleep(300000);
	}
}
