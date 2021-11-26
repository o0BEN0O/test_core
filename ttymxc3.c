#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

#define DEV_NAME "/dev/ttymxc3"
//#define DEV_NAME "/dev/vuart"
int SetOpt(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
	struct termios newtio,oldtio;
	if(tcgetattr(fd,&oldtio)!=0)
	{
		printf("error:SetupSerial %s %s\n",DEV_NAME,strerror(errno));
		return -1;
	}
	bzero(&newtio,sizeof(newtio));
	//使能串口接收
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	newtio.c_lflag &=~ICANON;//原始模式

	//newtio.c_lflag |=ICANON; //标准模式

	//设置串口数据位
	switch(nBits)
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |=CS8;
			break;
	}
	//设置奇偶校验位
	switch(nEvent)
	{
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E':
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':
			newtio.c_cflag &=~PARENB;
			break;
	}
	switch(nSpeed)
	{
		case 2400:
			cfsetispeed(&newtio,B2400);
			cfsetospeed(&newtio,B2400);
			break;
		case 4800:
			cfsetispeed(&newtio,B4800);
			cfsetospeed(&newtio,B4800);
			break;  
		case 9600:  
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);
			break;
		case 115200:
			cfsetispeed(&newtio,B115200);
			cfsetospeed(&newtio,B115200);
			break;
		case 460800:
			cfsetispeed(&newtio,B460800);
			cfsetospeed(&newtio,B460800);
			break;
		default:
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);
			break;
	}

	if(nStop == 1)
		newtio.c_cflag &= ~CSTOPB;
	else if(nStop == 2)
		newtio.c_cflag |= CSTOPB;

	newtio.c_cc[VTIME] = 1;		// 100ms
	newtio.c_cc[VMIN] = 16;
	tcflush(fd,TCIFLUSH);

	if(tcsetattr(fd,TCSANOW,&newtio)!=0)
	{
		printf("com set error %s %s\n",DEV_NAME,strerror(errno));
		return -1;
	}

	return 0;
}

void main(void){
	int fd;

	char buff[50] = {0};
	fd = open(DEV_NAME, O_RDWR | O_NOCTTY);
	if(fd < 0) 
	{
		printf("open %s failed %s\n",DEV_NAME,strerror(errno));
		close(fd);
		return;
	}

        char w_buff[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa};
        int read_size = 0, i = 0, write_size = 0;

	SetOpt(fd, 115200, 8, 'N', 1);
        while(1)
        {
                //write_size = write(fd, w_buff, sizeof(w_buff));
                //if(write_size < 0){
                //        perror("serial port write is error");
                //}

                read_size = read(fd, buff, sizeof(buff));
                printf("write_size = %d .receive length:%d data:", write_size, read_size);
                for(i = 0; i < read_size; i++){
                        printf("%02x ", buff[i]);
                }
                printf("\n");
                usleep(300000);
        }
}
