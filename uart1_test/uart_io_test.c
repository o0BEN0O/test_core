#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <time.h>
#include <stdarg.h>

/*!< Signed integer types  */
typedef   signed char     int8_t;
typedef   signed short    int16_t;


/*!< Unsigned integer types  */
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned long     uint32_t;

/*!< STM8Lx Standard Peripheral Library old types (maintained for legacy purpose) */

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t uc8;   /*!< Read Only */

typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

#define UART_DATA_BUFF_LEN		128

#define UART_PROTOCOL_LEN		9

#define UART_PAYLOAD_OFFSET	8

#define UART_DATA_MAX_LEN		32768

/*those id just for user，the real is AD_CHANNEL_ID*/
#define USER_HUB_CUR_ID 	1<<0
#define USER_HUB_VOL		1<<1
#define USER_HUB_TEMP		1<<2
#define USER_CAN_PORT_VOL	1<<3
#define USER_INPUT_IO1		1<<4
#define USER_INPUT_IO2		1<<5
#define USER_INPUT_IO3		1<<6
#define USER_INPUT_IO4		1<<7
#define USER_ALL_CHANNELS	0xff
/*those id just for user，the real is AD_CHANNEL_ID*/

/*those io channel just for user,the real is AD_OUTPUT_IO_CHANNEL_ID*/
#define USER_IO_CHANNEL_1	1<<0
#define USER_IO_CHANNEL_2	1<<1
#define USER_IO_CHANNEL_3	1<<2
#define USER_IO_CHANNEL_4	1<<3
/*those io channel just for user,the real is AD_OUTPUT_IO_CHANNEL_ID*/

#define HEAD_1 0x21
#define HEAD_2 0x50
#define HEAD_3 0x53

/* ad channel id
整机供电电流
整机供电电压
整机温度
CAN口电压
INPUT IO1电压
INPUT IO2电压
INPUT IO3电压
INPUT IO4电压
读取所有通道
ad channel id */

typedef enum{
	HUB_CUR=0x00,
	HUB_VOL=0x01,
	HUB_TEMP=0x02,
	CAN_PORT_VOL=0x03,
	INPUT_IO1=0x04,
	INPUT_IO2=0x05,
	INPUT_IO3=0x06,
	INPUT_IO4=0x07,
	MAX_AD_CHANNEL_ID,
	ALL_AD_CHANNEL=0xff,
}AD_CHANNEL_ID;

#define AD_DATE_PRE_LEN 3

/*tx command id*/
#define AD_DETECT_REQ 				0x0001
#define OUTPUT_LEVEL_CONFIG_REQ 	0x0002

/*rx_command id*/
#define AD_DETECT_ACK 				0x8001
#define AD_OUTPUT_LEV_CONF_UART1_ACK	0x8002
#define CONFIG_CONFIRM_ACK			0x8006

#define DEV_NAME  "/dev/ttymxc0"

typedef struct
{
	u8 id;
	u16 command;
	u16 Length;
}ImxPackageInfo_t;

typedef struct{
	u8 ad_channel_id;
	u16 channel_val;
}_ad_date_format;

static _ad_date_format ad_data[]=
{
	{HUB_CUR,0},
	{HUB_VOL,0},
	{HUB_TEMP,0},
	{CAN_PORT_VOL,0},
	{INPUT_IO1,0},
	{INPUT_IO2,0},
	{INPUT_IO3,0},
	{INPUT_IO4,0},
	{0xff,0xffff},
};

typedef enum{
	rx_status_init=-1,
	rx_status_date_invail=0,
	rx_status_date_empty,
	rx_status_done,
}UART_RX_STATUS_EM;


int fd;

FILE *log_fp;

int failed_cnt=0;

void get_time(char *buf)
{

	time_t t;
	t = time(NULL);
	struct tm *local = localtime(&t);
	snprintf(buf,20,"%d-%02d-%02d %02d:%02d:%02d",local->tm_year+1900,local->tm_mon+1,local->tm_mday,local->tm_hour,local->tm_min,local->tm_sec);
	return;
	/*
	ii = 1516020076
	璇锋寜浠绘剰?	*/
}

long get_sys_runtime(int type)
{
	struct timespec times = {0, 0};
	long time;
	clock_gettime(CLOCK_MONOTONIC, &times);
	//printf("CLOCK_MONOTONIC: %lu, %lu\n", times.tv_sec, times.tv_nsec);

	if (1 == type){
		time = times.tv_sec;
	}else{
		time = times.tv_sec * 1000 + times.tv_nsec / 1000000;
	}

	//JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "time = %ld\n", time);
    return time;
}


char *current_time(void){
	static char buf[64] = {0};
	struct tm *st;
	time_t tt;
	struct timeval  tv;
	gettimeofday (&tv, NULL);
	tt = tv.tv_sec;
    suseconds_t millitm;

	millitm = (tv.tv_usec + 500) / 1000;

	if (millitm == 1000) {
		++tt;
		millitm = 0;
	}

	st = localtime(&tt);
	st->tm_year += 1900;
	st->tm_mon += 1;
	sprintf(buf, "[%04d/%02d/%02d %02d:%02d:%02d:%03d] ", st->tm_year, st->tm_mon, st->tm_mday, st->tm_hour, st->tm_min, st->tm_sec, (int)millitm);
	return buf;
}



#define log_path "/raymarine/Data/uart_io_test.log"
int first_time_save_log=0;

void save_log(const char* format, ...)
{
	char time[32]="\0";
	char system_command[128] = "\0";
	char log_buf[128]="\0";
	char buf[256]="\0";
	if(first_time_save_log==0&&access(log_path,0)==0){
		snprintf(system_command,256,"echo -n \"%s\" >> %s","\n",log_path);
		system(system_command);
	}
	first_time_save_log++;
	va_list ap;
	va_start(ap,format);
	vsprintf(log_buf,format,ap);

	//get_time(time);
	snprintf(buf,256,"[%d]%s",get_sys_runtime(0),log_buf);

	//snprintf(system_command,256,"echo -n \"%s\" >> %s",buf,log_path);
	//system(system_command);
	fputs(buf,log_fp); \
	fflush(log_fp);
	va_end(ap);
}


int SetOpt(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
	struct termios newtio,oldtio;
	if(tcgetattr(fd,&oldtio)!=0)
	{
		//printf("error:SetupSerial %s %s\n",DEV_NAME,strerror(errno));
		return -1;
	}
	bzero(&newtio,sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	newtio.c_lflag &=~ICANON;

	//newtio.c_lflag |=ICANON;
	switch(nBits)
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |=CS8;
			break;
	}
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
		case 19200:
			cfsetispeed(&newtio,B19200);
			cfsetospeed(&newtio,B19200);
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

u8 cal_imx_package_checksum(u8 *pData,u16 nDataLen)
{
	u16 i;
	u8 u8Data;

	u8Data=0;
	for(i=0;i<nDataLen;i++)
	{
		u8Data=u8Data+pData[i];
	}
	u8Data=~u8Data+1;

	return u8Data;
}

#define AD_DATA_PRE_LEN 3
int ad_data_handle(uint8_t* rx_data,int id_cnt)
{
	int i=0;
	int j=0;
	uint8_t tmp_data[AD_DATA_PRE_LEN];
	for(i=0;i<id_cnt;i++)
	{
		tmp_data[0]=rx_data[i*AD_DATA_PRE_LEN];
		tmp_data[1]=rx_data[i*AD_DATA_PRE_LEN+1];
		tmp_data[2]=rx_data[i*AD_DATA_PRE_LEN+2];

		if(tmp_data[0]>MAX_AD_CHANNEL_ID)
			return rx_status_date_invail;

		for(j=0;j<MAX_AD_CHANNEL_ID;j++){
			if(ad_data[j].ad_channel_id==tmp_data[0]){
				ad_data[j].channel_val=0;//init to 0
				ad_data[j].channel_val=(tmp_data[2]<<8|tmp_data[1]);
				//JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW,"ad_data.id 0x%02x ad_data.val 0x%04x\n",ad_data[j].ad_channel_id,ad_data[j].channel_val);
			}
		}
	}
	return rx_status_done;
}


int uart_rx_protocol_process(u8 *rx_date,ImxPackageInfo_t rx_packinfo)
{
	int rx_status;
	switch(rx_packinfo.command){
		case AD_DETECT_ACK:
			rx_status=ad_data_handle(rx_date,rx_packinfo.Length/AD_DATE_PRE_LEN);
			break;
		case AD_OUTPUT_LEV_CONF_UART1_ACK:
			rx_status=rx_status_done;
			break;
		default:
			rx_status=rx_status_date_invail;
	}
	return rx_status;
}

UART_RX_STATUS_EM uart_rx(void)
{
	int i=0;
	int ret=-1;
	int rx_status=rx_status_init;
	u8 rx_buf[UART_DATA_BUFF_LEN];
	u8 rx_date_buf[UART_DATA_BUFF_LEN];
	u8 check_sum;
	int index=0;
	int timeout=100;
	char save_buf[UART_DATA_BUFF_LEN*5];
	char temp_buf[8];
	ImxPackageInfo_t rx_packinfo;

	memset(rx_buf,0,sizeof(rx_buf));
	memset(rx_date_buf,0,sizeof(rx_date_buf));
	memset(save_buf,0,sizeof(save_buf));
	while(1){
		ret=read(fd,rx_buf,UART_DATA_BUFF_LEN);
		if(ret>0)
			break;
		//printf("timeout %d\n",timeout);
		timeout+=100;
		if(timeout==200){
			printf("Receive data over time\n");
			rx_status=rx_status_date_empty;
			goto out;
		}
		usleep(100*1000);
	}

	if(rx_buf[index++]==HEAD_1&&rx_buf[index++]==HEAD_2&&rx_buf[index++]==HEAD_3)
	{
		rx_packinfo.id=rx_buf[index++];
		rx_packinfo.command=rx_buf[index++]|(rx_buf[index++]<<8);
		rx_packinfo.Length=rx_buf[index++]|(rx_buf[index++]<<8);
	}else{
		memset(save_buf,0,sizeof(save_buf));
		for(i=0;i<ret;i++){
			snprintf(temp_buf,8,"0x%02x ",rx_buf[i]);
			//printf("%s %d\n",temp_buf,strlen(temp_buf));
			memcpy(save_buf+i*5,temp_buf,5);
				//printf("%s \n",save_buf+i*4);
		}
		printf("Receive data checksum error,data:[ %s]\n",save_buf);
		rx_status=rx_status_date_invail;
		goto out;
	}

	memcpy(rx_date_buf,rx_buf+index,rx_packinfo.Length);

	index+=rx_packinfo.Length;
	check_sum=cal_imx_package_checksum(rx_buf+3,index-3);

	if(check_sum==rx_buf[index])
	{
		rx_status=uart_rx_protocol_process(rx_date_buf,rx_packinfo);
		if(rx_status!=rx_status_done){
			memset(save_buf,0,sizeof(save_buf));
			for(i=0;i<ret;i++){
					snprintf(temp_buf,8,"0x%02x ",rx_buf[i]);
					//printf("%s %d\n",temp_buf,strlen(temp_buf));
					memcpy(save_buf+i*5,temp_buf,5);
					//printf("%s \n",save_buf+i*4);
			}
			printf("Receive data checksum error,data:[ %s]\n",save_buf);
		}
	}
#if 0
	else{
		memset(save_buf,0,sizeof(save_buf));
		for(i=0;i<ret;i++){
				snprintf(temp_buf,8,"0x%02x ",rx_buf[i]);
				//printf("%s %d\n",temp_buf,strlen(temp_buf));
				memcpy(save_buf+i*5,temp_buf,5);
				//printf("%s \n",save_buf+i*4);
		}
		printf("Receive data checksum error,data:[ %s]\n",save_buf);
		rx_status=rx_status_date_invail;
		goto out;
	}
#endif
out:

	if(rx_buf[4]==0x15){
		memset(save_buf,0,sizeof(save_buf));
		for(i=0;i<ret;i++){
			snprintf(temp_buf,8,"0x%02x ",rx_buf[i]);
			memcpy(save_buf+i*5,temp_buf,5);
			//printf("%s \n",save_buf+i*4);
		}
		printf("%s \n",save_buf);
		//save_log("%s \n",save_buf);
	}


	return rx_status;
}

void uart_tx(ImxPackageInfo_t stPackageInfo, u8 *pData)
{
	u8 u8Uart1TxBuf[UART_DATA_BUFF_LEN+UART_PROTOCOL_LEN];
	u8 giTxLength;
	u8 giTxCnt;
	u8 u8Index;
	u8 u8Data;

	memset(u8Uart1TxBuf,0,sizeof(u8Uart1TxBuf));

	u8Index=0;

	u8Uart1TxBuf[u8Index++]=HEAD_1;
	u8Uart1TxBuf[u8Index++]=HEAD_2;
	u8Uart1TxBuf[u8Index++]=HEAD_3;
	u8Uart1TxBuf[u8Index++]=stPackageInfo.id;
	u8Uart1TxBuf[u8Index++]=stPackageInfo.command&0xFF;
	u8Uart1TxBuf[u8Index++]=(stPackageInfo.command>>8)&0xFF;
	u8Uart1TxBuf[u8Index++]=stPackageInfo.Length&0xFF;
	u8Uart1TxBuf[u8Index++]=(stPackageInfo.Length>>8)&0xFF;
	if(stPackageInfo.Length>128)
	{
		memcpy(&u8Uart1TxBuf[u8Index],pData,128);
		u8Index=u8Index+128;
	}
	else
	{
		memcpy(&u8Uart1TxBuf[u8Index],pData,stPackageInfo.Length);
		u8Index=u8Index+stPackageInfo.Length;
	}

	u8Data=cal_imx_package_checksum((u8Uart1TxBuf+3),(u8Index-3));
	u8Uart1TxBuf[u8Index++]=u8Data;

	giTxLength=u8Index;
	giTxCnt=1;
	int i=0;
	#if 0
	printf("tx_buf: ");
	for(i=0;i<u8Index;i++)
		printf("%02x ",u8Uart1TxBuf[i]);
	printf("\n");
	#endif
	//save_log("starting tx\n");
	write(fd,u8Uart1TxBuf,UART_PAYLOAD_OFFSET+stPackageInfo.Length+1);
	//save_log("ended tx\n");
	//perror("write\n");
}

int read_value_from_ad(uint8_t ad_id_set)
{
	int ret=rx_status_init;
	uint8_t index=0;
	uint8_t i=0;
	uint8_t date[UART_DATA_BUFF_LEN];
	ImxPackageInfo_t txPackageInfo;

	memset(date,0,sizeof(date));

	txPackageInfo.id=0;
	txPackageInfo.command=0x0001;

	if(ad_id_set==USER_ALL_CHANNELS)
		date[index++]=USER_ALL_CHANNELS;
	else{
		for(i=0;i<8;i++){
			if(ad_id_set&(1<<i))
				date[index++]=i;
		}
	}

	txPackageInfo.Length=index;

	//tcflush(fd, TCOFLUSH);

	//ioctl(fd, TCFLSH, 1);

	uart_tx(txPackageInfo,date);

	ret=uart_rx();

	return ret;
}

void Stop(int signo)
{
	tcflush(fd,TCIOFLUSH);

	ioctl(fd, TCFLSH, 2);

	printf("close fd\n");
    close(fd);
	fclose(log_fp);
    _exit(0);
}


UART_RX_STATUS_EM ad_output_level_set(uint8_t io_sets,uint8_t ouptut_value)
{
	uint8_t i=0;
	uint8_t index=0;
	int ret=rx_status_init;
	uint8_t data[UART_DATA_BUFF_LEN];
	ImxPackageInfo_t txPackageInfo;
	txPackageInfo.id=0;
	txPackageInfo.command=OUTPUT_LEVEL_CONFIG_REQ;
	data[index++]=0x01;
	for(i=0;i<4;i++){
		if(io_sets&(1<<i)){
			data[index++]=i;
			data[index++]=ouptut_value;
		}
	}
	txPackageInfo.Length=index;


	uart_tx(txPackageInfo,data);

	ret=uart_rx();

	return ret;
}

int main (int argc, char *argv[])
{

	int len;
	int i=0;
    char buf[32];
	unsigned char tx_buf[8]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x00};
	ImxPackageInfo_t ad_detect;
	int time_out=100;
	int rx_status=rx_status_init;
	double volt_val;

	u8 date[UART_DATA_BUFF_LEN];

	log_fp = fopen(log_path,"wt+");


#if 0
	uart_tx(ad_detect,date);

	read_value_from_ad(USER_HUB_CUR_ID|USER_HUB_VOL|USER_HUB_TEMP, ad_date_set);
	ad_output_level_set(USER_IO_CHANNEL_1|USER_IO_CHANNEL_2,1);
	ad_output_level_set(USER_IO_CHANNEL_3|USER_IO_CHANNEL_4,0);
	ad_output_level_set(USER_IO_CHANNEL_1|USER_IO_CHANNEL_2|USER_IO_CHANNEL_3|USER_IO_CHANNEL_4,0);
	ad_output_level_set(USER_IO_CHANNEL_1|USER_IO_CHANNEL_2|USER_IO_CHANNEL_3|USER_IO_CHANNEL_4,1);
	return 0;
#endif
	signal(SIGINT, Stop);

	fd = open(DEV_NAME, O_RDWR|O_NOCTTY|O_NDELAY);
	if(fd < 0) {
		printf("IO OPEN UART FAILED\n");
		return -1;
	}
	//SetOpt(fd, 115200, 8, 'N', 1);
	SetOpt(fd, 19200, 8, 'N', 1);
	//ad_output_level_set(USER_IO_CHANNEL_1|USER_IO_CHANNEL_2|USER_IO_CHANNEL_3|USER_IO_CHANNEL_4,1);


	while(1){
		rx_status=read_value_from_ad(ALL_AD_CHANNEL);
		usleep(500*1000);
	}
#if 0
	if(rx_status==rx_status_done){
		for(i=0;i<4;i++){
			volt_val=(double)(ad_data[INPUT_IO1+i].channel_val/10);
			if(volt_val<2){
				save_log("input_vol:[%dv] is less than 2v\n",volt_val);
				printf("IO %d DETECT VOL FAILED\n",i+1);
			}else{
				printf("IO %d TEST SUCC\n",i+1);
			}
		}
	}else{
		printf("IO RECEIVE DATA FAILED\n");
	}

	do{
		rx_status = uart_rx();
		if(rx_status == rx_status_done){
			rx_status=rx_status_init;
			break;
		}
		time_out--;
		usleep(100*1000*1000);
	}while(time_out);
#endif
	//ad_output_level_set(USER_IO_CHANNEL_1|USER_IO_CHANNEL_2|USER_IO_CHANNEL_3|USER_IO_CHANNEL_4,1);
	close(fd);
	return(0);
}


