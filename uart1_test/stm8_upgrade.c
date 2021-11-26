#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <sys/stat.h>

const unsigned short Crc16CCITtable[] =
{
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
        0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
        0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738,
        0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
        0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
        0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb,
        0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2,
        0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
        0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827,
        0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,
        0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
        0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

unsigned short Crc16CCITCalculate(unsigned char const *pData,unsigned long lDataLength,unsigned short crc)
{
        unsigned short ushort;
        //unsigned short crc = 0;
        int i;

#if 0
        if(lDataLength > MAX_IMAGE_SIZE)
        {
                return 0;
        }
#endif
        for(i = 0;i < lDataLength;i ++)
        {
                ushort = (crc << 8) & 0xff00;
                crc = ((ushort) ^ Crc16CCITtable[((crc >> 8) ^ (0xff & *pData))]);
                pData ++;
        }
        return crc;
}



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

int failed_cnt=0;

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

u8 cal_stm8_upgrade_package_checksum(u8 *pData,u16 nDataLen)
{
	u16 i;
	u8 u8Data;

	u8Data=0;
	for(i=0;i<nDataLen;i++)
	{
		//printf("%02x ",pData[i]);
		u8Data^=pData[i];
	}
	//printf("\n");
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

int confirm_set_ack(uint8_t* rx_date,uint16_t tx_command)
{
	if(tx_command == (rx_date[1]<<8|rx_date[0])){
		printf("set done ack: 0x%04x\n",rx_date[1]<<8|rx_date[0]);
		return rx_status_done;
	}else{
		printf("tx_command 0x%04x ACK: 0x%04x\n",tx_command,rx_date[1]<<8|rx_date[0]);
		return rx_status_date_invail;
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

	int ret=-1;
	int rx_status=rx_status_init;
	u8 rx_buf[UART_DATA_BUFF_LEN]={0x00};
	u8 rx_date_buf[UART_DATA_BUFF_LEN]={0x00};

	u8 check_sum;

	int index=0;

	int timeout=100;

	//printf("uart_rx\n");

	ImxPackageInfo_t rx_packinfo;

	while(ret==-1){
		ret=read(fd,rx_buf,UART_DATA_BUFF_LEN);
		//printf("timeout %d\n",timeout);
		//timeout+=100;
		//usleep(10*1000);
	}
	//perror("read");
	//printf("ret %d\n",ret);

	int i=0;
	//printf("rx_buf:\n");
	if(ret==-1){
		++failed_cnt;
		printf("recv failed cnt %d\n",failed_cnt);
		return 0;
	}
	//printf("rx_buf: ");
	//for(i=0;i<ret;i++){
	//	printf("%02x ",rx_buf[i]);
	//}
	//printf("\n");

	return rx_status_done;

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

	//printf("rx_buf:\n");
	//for(i=0;i<index;i++){
	//	printf("%02x ",rx_buf[i]);
	//}
	///printf("\n");

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
	printf("tx_buf: ");
	for(i=0;i<u8Index;i++)
		printf("%02x ",u8Uart1TxBuf[i]);
	printf("\n");
	write(fd,u8Uart1TxBuf,UART_PAYLOAD_OFFSET+stPackageInfo.Length+1);
	//perror("write\n");
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

int stm8_upgrade_rsp(void)
{
	usleep(10*1000);
	return 0;

	int ret=-1;
	int timeout=0;
	uint8_t rx_date;
	//_modbus_receive_msg(ctx,&rx_date,1);
	while(ret==-1){
		ret=read(fd,&rx_date,UART_DATA_BUFF_LEN);
		//printf("timeout %d\n",timeout);
		//timeout+=100;
		//usleep(100*1000);
	}

	if(rx_date!=0x79){
		printf("return error %2x\n",rx_date);
		return -1;
	}
	return 0;
}

int stm8_upgrade_program(char *mcu_filename)
{
	int i=0;
	int j=0;
	int k=0;
	int index=0;
	int ret=-1;
	int date_length=0;
	char head_info[16];
	char *date_buf;
	int checksum=-1;
	FILE *mcu_bin;
	int mcu_bin_size=0;
	int image_offset_size=0;
	int pre_date_size=0;
	//uint32_t addr=0x00009ff0;
	uint32_t addr=0x0000bff0;
	uint8_t start_date=0x7f;
	uint8_t head_date[2]={0x31,0xce};
	uint8_t end_date[2]={0x21,0xde};
	uint8_t restart_date[5]={0x00,0x00,0xa0,0x00,0xa0};
	//uint8_t restart_date[5]={0x00,0x00,0x90,0x00,0x90};
	uint8_t package_head[5];
	uint8_t package_date[130];
	int zero_num=0;
	#if 0
	write(fd,end_date,2);
	if(stm8_upgrade_rsp()!=0){
		printf("4\n");
		fclose(mcu_bin);
		return -1;
	}

	write(fd,restart_date,5);
	if(stm8_upgrade_rsp()!=0){
		printf("5\n");
		fclose(mcu_bin);
		return -1;
	}
	return 0;
	#endif

	#if 0
	printf("enter stm8_upgrade_program");
	mcu_bin = fopen(mcu_filename,"r");
	if(mcu_bin==NULL){
		printf("open failed\n");
		return -1;
	}

	ret=fread(head_info,1,16,mcu_bin);
	if(ret!=16){
		printf("%s format error\n",mcu_filename);
		fclose(mcu_bin);
		return -1;
	}
	//for(i=0;i<16;i++)
		//printf("%02x ",head_info[i]);
	//printf("\n");
	checksum=head_info[5]<<8|head_info[4];
	mcu_bin_size=head_info[9]<<8|head_info[8];
	//printf("checksum %d mcu_bin_size %d\n",checksum,mcu_bin_size);
	if(fseek(mcu_bin,16,SEEK_SET)!=0){
		printf("%s format error\n",mcu_filename);
		fclose(mcu_bin);
		return -1;
	}
	date_buf=malloc(mcu_bin_size);
	ret=fread(date_buf,1,mcu_bin_size,mcu_bin);
	if(ret!=mcu_bin_size){
		printf("%s format error\n",mcu_filename);
		free(date_buf);
		fclose(mcu_bin);
		return -1;
	}
	if(checksum!=Crc16CCITCalculate(date_buf,mcu_bin_size,0)){
		printf("%s format error\n",mcu_filename);
		printf("checksum %d cal_checksum %d\n",checksum,Crc16CCITCalculate(date_buf,mcu_bin_size,0));
		free(date_buf);
		fclose(mcu_bin);
		return -1;
	}
	free(date_buf);

	mcu_bin_size+=16;

	ImxPackageInfo_t ad_detect;

	ad_detect.id = 0x00;
	ad_detect.command = 0x03;
	ad_detect.Length = 1;
	u8 date[UART_DATA_BUFF_LEN]={0x01};
	int rx_status;

	uart_tx(ad_detect,date);
	rx_status=uart_rx();
	if(rx_status != rx_status_done){
		printf("uart rx failed\n");
		close(fd);
		return 0;
	}
	usleep(700*1000);
	write(fd,&start_date,1);
	if(stm8_upgrade_rsp()!=0){
		printf("0\n");
		fclose(mcu_bin);
		return -1;
	}
	//printf("1.enter stm8_upgrade_program\n");

	for(i=0;image_offset_size<mcu_bin_size;i++){
		write(fd,head_date,2);
		if(stm8_upgrade_rsp()!=0){
			printf("1\n");
			fclose(mcu_bin);
			return -1;
		}
		for(j=0;j<5;j++){
			if(j<4)
				package_head[j]=(addr>>(8*(3-j))&0xff);
			else
				package_head[j]=cal_stm8_upgrade_package_checksum(package_head,4);
		}
		write(fd,package_head,5);
		if(stm8_upgrade_rsp()!=0){
			printf("2\n");
			fclose(mcu_bin);
			return -1;
		}
		bzero(package_date,130);
		index=0;

		if((mcu_bin_size-image_offset_size)<128)
			date_length=mcu_bin_size-image_offset_size;
		else
			date_length=128;

		//printf("date_length %d\n",date_length);

		//if(date_length<128){
		//	zero_num=date_length%4;
		//	date_length+=zero_num;
		//}
		package_date[index++]=date_length-1;
		#if 0
		if(fseek(mcu_bin,image_offset_size,SEEK_SET)!=0){
				perror("fseek error\n");
				fclose(mcu_bin);
				return -1;
		}
		ret=fread(package_date+index,1,date_length,mcu_bin);
		if(ret!=date_length){
				fclose(mcu_bin);
				return -1;
		}
		#endif
		index+=date_length;

		if(date_length<128&&date_length%4){
			zero_num=(4-(date_length%4));
			//printf("zeronum %d\n",zero_num);
			package_date[0]=(zero_num+date_length)-1;
			for(k=0;k<zero_num;k++)
				package_date[index++]=0x00;
		}

		package_date[index++]=cal_stm8_upgrade_package_checksum(package_date,date_length+1);

		//for(k=0;k<index;k++)
		//	printf("%02x ",package_date[k]);
		//printf("\n");

		write(fd,package_date,index);
		if(stm8_upgrade_rsp()!=0){
			printf("3\n");
			fclose(mcu_bin);
			return -1;
		}
		addr+=date_length;
		image_offset_size+=date_length;
		//printf("offset %d,total%d\n",image_offset_size,mcu_bin_size);
	}
		printf("enter stm8_upgrade_program");
#endif

	write(fd,end_date,2);
	if(stm8_upgrade_rsp()!=0){
		printf("4\n");
		//fclose(mcu_bin);
		return -1;
	}

		printf("enter stm8_upgrade_program");

	write(fd,restart_date,5);
	if(stm8_upgrade_rsp()!=0){
		printf("5\n");
		//fclose(mcu_bin);
		return -1;
	}

	return 0;
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
	int time_out=100;
	int rx_status=rx_status_init;
	ad_detect.id = 0x00;
	ad_detect.command = 0x03;
	ad_detect.Length = 1;
	u8 date[UART_DATA_BUFF_LEN]={0x01};
	char file_path[256]="\0";
	//u8 date[UART_DATA_BUFF_LEN]={0x01,0x7d,0x00,0x00,0xb1,0x0d,0x02,0xc3,0x0a};
	//u8 rx_buf[UART_DATA_BUFF_LEN]={0x00};
	//_ad_date_format ad_date_set[8];
	strcpy(file_path,argv[1]);

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
		perror(DEV_NAME);
		return -1;
	}
	//SetOpt(fd, 115200, 8, 'N', 1);
	SetOpt(fd, 19200, 8, 'N', 1);
	#if 1
	#endif
	stm8_upgrade_program(file_path);
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
	close(fd);
	return(0);
}


