#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>

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

static _ad_date_format ad_date[]=
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

int SetOpt(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
	struct termios newtio,oldtio;
	if(tcgetattr(fd,&oldtio)!=0)
	{
		printf("error:SetupSerial %s %s\n",DEV_NAME,strerror(errno));
		return -1;
	}
	bzero(&newtio,sizeof(newtio));
	//浣胯兘涓插彛鎺ユ敹
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	newtio.c_lflag &=~ICANON;//鍘熷妯″紡

	//newtio.c_lflag |=ICANON; //鏍囧噯妯″紡

	//璁剧疆涓插彛鏁版嵁浣?
	switch(nBits)
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |=CS8;
			break;
	}
	//璁剧疆濂囧伓鏍￠獙浣?
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


void ad_date_handle(u8* rx_date,int id_cnt)
{
	int i=0;
	int j=0;
	//_ad_date_format tmp_date={0x00,0x0000};
	for(i=0;i<id_cnt;i++)
	{
		//memcpy(tmp_date,rx_date[i*AD_DATE_PRE_LEN],AD_DATE_PRE_LEN);
		for(j=0;j<MAX_AD_CHANNEL_ID;j++){
			//if(ad_date[j].ad_channel_id==tmp_date.ad_channel_id)
			//	ad_date[j].channel_val=tmp_date.channel_val;
		}
		//tmp_date={0x00,0x0000};
	}
}

uart_rx_protocol_process(u8 *rx_date,ImxPackageInfo_t rx_packinfo)
{
	switch(rx_packinfo.command){
		case AD_DETECT_ACK:
			ad_date_handle(rx_date,rx_packinfo.Length/AD_DATE_PRE_LEN);
			break;
		case CONFIG_CONFIRM_ACK:
			break;
	}
}

UART_RX_STATUS_EM uart_rx(void)
{
	int ret=0;
	int rx_status=rx_status_init;
	u8 rx_buf[UART_DATA_BUFF_LEN]={0x00};
	u8 rx_date_buf[UART_DATA_BUFF_LEN]={0x00};

	u8 check_sum;
	
	int index=0;

	printf("uart_rx\n");

	ImxPackageInfo_t rx_packinfo;

	ret=read(fd,rx_buf,UART_DATA_BUFF_LEN);
	perror("read");
	printf("ret %d\n",ret);

	int i=0;
	printf("rx_buf:\n");
	for(i=0;i<ret;i++){
		printf("%02x ",rx_buf[i]);
	}
	printf("\n");

	if(ret!=UART_PAYLOAD_OFFSET){
		rx_status=rx_status_date_empty;
		goto out;
	}

	if(rx_buf[0]==HEAD_1&&rx_buf[1]==HEAD_2&&rx_buf[2]==HEAD_3)
	{
		rx_packinfo.id=rx_buf[index++];
		rx_packinfo.command=rx_buf[index++]|(rx_buf[index++]<<8);
		rx_packinfo.Length=rx_buf[index++]|(rx_buf[index++]<<8);
	}else{
		rx_status=rx_status_date_invail;
		goto out;
	}
	ret=read(fd,rx_buf+index,rx_packinfo.Length);
	if(ret != rx_packinfo.Length){
		rx_status=rx_status_date_invail;
		goto out;
	}
	memcpy(rx_date_buf,rx_buf+index,rx_packinfo.Length);

	index+=rx_packinfo.Length;
	check_sum=cal_imx_package_checksum(rx_buf+3,index-3);

	ret = read(fd,rx_buf+index,1);
	if(ret!=1){
		rx_status=rx_status_date_invail;
		goto out;
	}

	index++;

	printf("rx_buf:\n");
	for(i=0;i<index;i++){
		printf("%02x ",rx_buf[i]);
	}
	printf("\n");

	if(check_sum==rx_buf[index])
	{
		uart_rx_protocol_process(rx_date_buf,rx_packinfo);
	}else{
		rx_status=rx_status_date_invail;
		goto out;
	}
out:
	printf("error rx_status %d\n",rx_status);
	close(fd);
	return rx_status;
}

void uart_tx(ImxPackageInfo_t stPackageInfo, u8 *pData)
{
	u8 u8Uart1TxBuf[UART_DATA_BUFF_LEN+UART_PROTOCOL_LEN]={0x00};
	u8 giTxLength;
	u8 giTxCnt;
	u8 u8Index;
	u8 u8Data;

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
	for(i=0;i<u8Index;i++)
		printf("%02x ",u8Uart1TxBuf[i]);
	printf("\n");
	write(fd,u8Uart1TxBuf,UART_PAYLOAD_OFFSET+stPackageInfo.Length+1);
	perror("write\n");
}

int read_value_from_ad(uint8_t ad_id_set,_ad_date_format *ad_date_set)
{
	int ret=rx_status_init;
	uint8_t index=0;
	uint8_t i=0;
	uint8_t date[UART_DATA_BUFF_LEN];
	ImxPackageInfo_t txPackageInfo;

	txPackageInfo.id=0;
	txPackageInfo.command=0x0001;
	for(i=0;i<8;i++){
		if(ad_id_set&(1<<i))
			date[index++]=i;
	}
	txPackageInfo.Length=index;
	uart_tx(txPackageInfo,date);
	return 0;
	if(ret!=rx_status_done)
		return ret;
#if 0
	for(i=0;i<8;i++){
		if(ad_id_set&(1<<i)){
			ad_date_set[i].ad_channel_id=ad_date[i].ad_channel_id;
			ad_date_set[i].channel_val=ad_date[i].channel_val;
		}
	}
#endif
	return ret;
}

UART_RX_STATUS_EM ad_output_level_set(uint8_t io_sets,uint8_t ouptut_value)
{
	uint8_t i=0;
	uint8_t index=0;
	int ret=rx_status_init;
	uint8_t date[UART_DATA_BUFF_LEN];
	ImxPackageInfo_t txPackageInfo;
	txPackageInfo.id=0;
	txPackageInfo.command=0x0002;
	for(i=0;i<4;i++){
		if(io_sets&(1<<i)){
			date[index++]=i;
			date[index++]=ouptut_value;
		}
	}
	txPackageInfo.Length=index;
	uart_tx(txPackageInfo,date);

	return ret;
}

void Stop(int signo) 
{
	printf("close fd\n");
    close(fd);
    _exit(0);
}



int main (int argc, char *argv[])
{

	int len;
	int i=0;
    char buf[32];
	unsigned char tx_buf[8]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x00};
	ImxPackageInfo_t ad_detect;
	int time_out=10;
	int rx_status=rx_status_init;
	ad_detect.id = 0x00;
	ad_detect.command = 0x0004;
	ad_detect.Length = 1;
	u8 date[UART_DATA_BUFF_LEN]={0x00};
	//u8 date[UART_DATA_BUFF_LEN]={0x01,0x7d,0x00,0x00,0xb1,0x0d,0x02,0xc3,0x0a};
	//u8 rx_buf[UART_DATA_BUFF_LEN]={0x00};
	//_ad_date_format ad_date_set[8];

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

	fd = open(DEV_NAME, O_RDWR|O_NOCTTY);
	if(fd < 0) {
		perror(DEV_NAME);
		return -1;
	}
	SetOpt(fd, 115200, 8, 'N', 1);
	uart_tx(ad_detect,date);
	rx_status = uart_rx();
#if 0
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
	return(0);
}
 

