#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include <stdbool.h>

#include <time.h>
#include <stdarg.h>


#include <stdint.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/ioctl.h>



/*!< Signed integer types  */
typedef   signed char		int8;
typedef   signed short		int16;
typedef   signed long		int32;


/*!< Unsigned integer types  */
typedef unsigned char     uint8;
typedef unsigned short    uint16;
typedef unsigned long     uint32;

/*!< STM8Lx Standard Peripheral Library old types (maintained for legacy purpose) */

typedef const uint32 uc32;  /*!< Read Only */
typedef const uint16 uc16;  /*!< Read Only */
typedef const uint8 uc8;   /*!< Read Only */

typedef int32  s32;
typedef int16 s16;
typedef int8  s8;

typedef uint32  u32;
typedef uint16 u16;
typedef uint8  u8;

#define UART_DATA_BUFF_LEN		1024

#define TRUE	1
#define FALSE	0

//#define DEV_NAME  "/dev/ttymxc0"
#define DEV_NAME  "/dev/ttymxc2"

#define RMC_TEST_DATA "$GNRMC,094449.00,A,2241.52300,N,11417.56819,E,0.075,,241220,,,D*6A"


int fd;

typedef struct
{
	u16 Year;
	u8 Month;
	u8 Day;
	u8 Hour;
	u8 Minute;
	u16 Second;

	u8 cLatitudeDegree;	//纬度:度数
	u32 lLatitudeMin;	//纬度:分数 扩大100000倍，实际值需除以100000
	u8 cSouthNorth;		 //N:北纬，S南纬

	u8 cLongitudeDegree;	//经度:度数
	u32 lLongitudeMin;	//经度:分数 扩大100000倍，实际值需除以100000
	u8 cEastWest;		//E:东经，W:西经

	s32 sAltitude;	 //海拔高度，放大了10倍，实际值需要除以10 单位为0.1m
	u16 speed;		//地面速率，放大了10倍，0.1knots，1knots = 1.852 km/h
	u16 cog;			//航向角度，1度
	//纬度:度数
	//纬度:分数 扩大100000倍，实际值需除以100000
	//N:北纬，S南纬

	//经度:度数
	//经度:分数 扩大100000倍，实际值需除以100000
	//E:东经，W:西经
	int quality;
	u8 NumofSv;
	s32 sGeoidalSeparation;
	s16 sHDOP;
	s16 sPDOP;
	u16 GpsDiffAge;
	u16 GpsDiffId;
	u16 GlonassDiffAge;
	u16 GlonassDiffId;
	u8 Integrity;
	u8 FixMode;		//定位类型 1:未定位 2:2D定位 3:3D定位
	u16 mv;
	u8 mvEW;		//M cog = cog + mvW or cog - mvE
	u8 bGPSFix;
	u8 bGLNOASSFix;
}NMEA0183_Gps_t;

typedef enum
{
	E_NMEA0183_VTG=0,
	E_NMEA0183_GGA,
	E_NMEA0183_GNS,
	E_NMEA0183_GSA,
	E_NMEA0183_GSV,
	E_NMEA0183_GST,
	E_NMEA0183_GLL,
	E_NMEA0183_GRS,
	E_NMEA0183_ZDA,
	E_NMEA0183_DTM,
	E_NMEA0183_RMC,
	E_NMEA0183_RMA,

	E_NMEA0183_NULL,
}E_NMEA0183_DATA_TYPE;


NMEA0183_Gps_t pst0183GpsData;

void get_time(char *buf)
{

	time_t t;
	t = time(NULL);
	struct tm *local = localtime(&t);
	snprintf(buf,20,"%d-%02d-%02d %02d:%02d:%02d",local->tm_year+1900,local->tm_mon+1,local->tm_mday,local->tm_hour,local->tm_min,local->tm_sec);
	return;
}


#define log_path "/raymarine/Data/gps_uart_test.log"
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

	get_time(time);
	snprintf(buf,256,"[%s]%s",time,log_buf);

	snprintf(system_command,256,"echo -n \"%s\" >> %s",buf,log_path);
	system(system_command);
	va_end(ap);
}



u8 nmea0183_star_Pos(int8 *buf,int8 cx, u16 nMaxLen)
{
	int8 *p=buf;
	while(cx)
	{
		if((*buf<' ')||(*buf>'z'))
		{
			return 0XFF;
		}
		if(*buf=='*')
		{
			cx--;
		}
		buf++;
		nMaxLen--;
		if(nMaxLen==0)
		{
			break;
		}
	}
	if(cx==0)
	{
		return buf-p;
	}
	else
	{
		return 0XFF;
	}
}

u8 nmea0183_get_checksum(u8 *buf)
{
	u8 cTempData;
	if( (*buf >= '0') && (*buf <= '9') )
	{
		cTempData = *buf - '0';
	}
	else if( (*buf >= 'A') && (*buf <= 'F') )
	{
		cTempData = *buf - 'A' + 10;
	}
	else
	{
		return 0xFF;
	}

	cTempData <<= 4;

	if( (*(buf + 1) >= '0') && (*(buf + 1) <= '9') )
	{
		cTempData |= (*(buf + 1) - '0');
	}
	else if( (*(buf + 1) >= 'A') && (*(buf + 1) <= 'F') )
	{
		cTempData |= (*(buf + 1) - 'A' + 10);
	}
	else
	{
		return 0xFF;
	}
	return cTempData;
}

u8 bcc_checksum(u8 *buf, u8 len)
{
	int8 ret=0,i=0;
	for(i=0;i<len;i++)
	{
		ret ^= *(buf+i);
	}
	return ret;
}

#if 1
u8 find_nmea0183_head(u8 *buf1, u16 nRxLen)
{
	u16 i;

	for(i=0;i<nRxLen;i++)
	{
		if(*(buf1+i)=='$')
		{
			if(i<(nRxLen-6))
			{
				switch(*(buf1+i+3))//加3为了去掉$GN或者$GP
				{
					case 'V':
						if(*(buf1+i+4)=='T' && *(buf1+i+5)=='G')
						{
							return E_NMEA0183_VTG;
						}
						else
						{
							return E_NMEA0183_NULL;
						}
					break;

					case 'G':
						if(*(buf1+i+4)=='G' && *(buf1+i+5)=='A')
						{
							return E_NMEA0183_GGA;
						}
						else if(*(buf1+i+4)=='N' && *(buf1+i+5)=='S')
						{
							return E_NMEA0183_GNS;
						}
						else if(*(buf1+i+4)=='S' && *(buf1+i+5)=='A')
						{
							return E_NMEA0183_GSA;
						}
						else if(*(buf1+i+4)=='S' && *(buf1+i+5)=='V')
						{
							return E_NMEA0183_GSV;
						}
						else if(*(buf1+i+4)=='S' && *(buf1+i+5)=='T')
						{
							return E_NMEA0183_GST;
						}
						else if(*(buf1+i+4)=='L' && *(buf1+i+5)=='L')
						{
							return E_NMEA0183_GLL;
						}
						else if(*(buf1+i+4)=='R' && *(buf1+i+5)=='S')
						{
							return E_NMEA0183_GRS;
						}
						else
						{
							return E_NMEA0183_NULL;
						}
					break;

                                case 'Z':
						if(*(buf1+i+4)=='D' && *(buf1+i+5)=='A')
						{
							return E_NMEA0183_ZDA;
						}
						else
						{
							return E_NMEA0183_NULL;
						}
					break;

					case 'D':
						if(*(buf1+i+4)=='T' && *(buf1+i+5)=='M')
						{
							return E_NMEA0183_DTM;
						}
						else
						{
							return E_NMEA0183_NULL;
						}
					break;

					case 'R':
						if(*(buf1+i+4)=='M' && *(buf1+i+5)=='C')
						{
							return E_NMEA0183_RMC;
						}
						else if(*(buf1+i+4)=='M' && *(buf1+i+5)=='A')
						{
							return E_NMEA0183_RMA;
						}
						else
						{
							return E_NMEA0183_NULL;
						}
					break;

					default:
						return E_NMEA0183_NULL;
					break;
				}
			}
		}
	}
	return E_NMEA0183_NULL;
}
#endif

//返回cx个逗号所在位置
static int8 NMEA_Comma_Pos(int8 *buf,int8 cx,u16 nMaxLen)
{
	int8 *p=buf;
	while(cx)
	{
		if(*buf=='*'||*buf<' '||*buf>'z')
		{
			return 0XFF;
		}
		if(*buf==',')
		{
			cx--;
		}
		buf++;
		nMaxLen--;
		if(nMaxLen==0)
		{
			break;
		}

	}
	if(cx==0)
	{
		return buf-p;
	}
	else
	{
		return 0XFF;
	}
}

//计算m的n次方
static uint32 NMEA_Pow(int8 m,int8 n)
{
	uint32 result=1;
	while(n--)result*=m;
	return result;
}

static int NMEA_Str2num(int8 *buf,int8 *dx,int8 cMaxFlen,int8 *bBlank)
{
	int8 *p=buf;
	uint32 ires=0,fres=0;
	int8 ilen=0,flen=0,i;
	int8 mask=0;
	int res;
	while(1)
	{
		if(*p=='-')
		{
			mask|=0X02;
			p++;
		}
		if((*p==',')||(*p=='*'))
		{
			break;
		}
		if(*p=='.')
		{
			mask|=0X01;
			p++;
		}
		else if((*p>'9')||(*p<'0'))
		{
			ilen=0;
			flen=0;
			break;
		}
		if(mask&0X01)
		{
			flen++;
		}
		else
		{
			ilen++;
		}
		p++;
	}
	if((ilen==0)&&(flen==0))
	{
		*bBlank=TRUE;
	}
	else
	{
		*bBlank=FALSE;
	}
	if(mask&0X02)
	{
		buf++;
	}
	for(i=0;i<ilen;i++)
	{
		ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
	}
	if(flen>cMaxFlen)
	{
		flen=cMaxFlen;
	}
	*dx=flen;
	for(i=0;i<flen;i++)
	{
		fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
	}
	res=ires*NMEA_Pow(10,flen)+fres;

	if(flen < cMaxFlen && flen > 0)		//四舍五入
	{
		if((buf[ilen+flen+1] - '0') > 5)
		{
			res += 1;
		}
	}

	if(mask&0X02)
	{
		res=-res;
	}

	return res;
}

void api_nmea0183_rmc_analysis(u8 *buf, bool bInternalModule,u16 nRxLen)
{
	u8 p1[nRxLen];
	u8 dx = 0;
	u8 posx;
	u32 temp;
	u8 cCheckSum;
	//NMEA0183_Gps_t *pst0183GpsData;
	u8 bBlank;
	u8 cPosMode;
	//GpsPosition_T stLocal;

	//p1=malloc(nRxLen);
	memset(p1,0,nRxLen);
	memset(&pst0183GpsData,0,sizeof(NMEA0183_Gps_t));
	memcpy(p1,buf,nRxLen);
	posx = nmea0183_star_Pos(p1,1,nRxLen);
	if(posx!=0xFF)
	{
		cCheckSum = nmea0183_get_checksum(p1 + posx);
		if(cCheckSum != bcc_checksum((p1 + 1), (posx - 2)) )
		{
			return;
		}
	}
	else
	{
		return;
	}
#if 0
	bGpsDataReceived = TRUE;
	nGpsDataRecived = 0;
	if(bInternalModule)
	{
		cNmea0183InternalDataCnt = 0;
		pst0183GpsData = &stInternal0183GpsData;
	}
	else
	{
		cNmea0183externalDataCnt = 0;
		pst0183GpsData = &stExNmea0183GpsData;
	}
#endif
	//UTC时间
	posx=NMEA_Comma_Pos(p1,1,nRxLen);
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx,2,&bBlank);
		if(dx < 2)
		{
			temp = temp * NMEA_Pow(10,(2-dx));
		}
		pst0183GpsData.Hour=temp/1000000;
		pst0183GpsData.Minute=(temp/10000)%100;
		pst0183GpsData.Second=temp%10000;	 	 //秒放大100倍，取到小数点后2位
	}

	//定位状态
	posx=NMEA_Comma_Pos(p1,2,nRxLen);
	if(posx!=0XFF)
	{
		cPosMode=*(p1+posx);

		if(cPosMode=='A')//
		{
			pst0183GpsData.quality=1;
		}
		else if(cPosMode=='V')
		{
			pst0183GpsData.quality=2;
		}
		else
		{
			pst0183GpsData.quality=0;
		}
	}

	//获得纬度
	posx=NMEA_Comma_Pos(p1,3,nRxLen);
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx,5,&bBlank);
		if(!bBlank)
		{
			if(dx < 5)
			{
				temp = temp * NMEA_Pow(10,(5-dx));
			}

			pst0183GpsData.cLatitudeDegree = temp/NMEA_Pow(10,5+2);	//计算度

			pst0183GpsData.lLatitudeMin = temp%NMEA_Pow(10,5+2);  //计算分，放大100000倍
		}
	}

	//纬度半球
	posx=NMEA_Comma_Pos(p1,4,nRxLen);
	if(posx!=0XFF)
	{
		pst0183GpsData.cSouthNorth = *(p1+posx);
	}

 	//获得经度
	posx=NMEA_Comma_Pos(p1,5,nRxLen);
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx,5,&bBlank);
		if(!bBlank)
		{
			if(dx < 5)
			{
				temp = temp * NMEA_Pow(10,(5-dx));
			}

			pst0183GpsData.cLongitudeDegree = temp/NMEA_Pow(10,5+2);	//计算度

			pst0183GpsData.lLongitudeMin = temp%NMEA_Pow(10,5+2);  //计算分，放大100000倍
		}
	}
	//经度半球
	posx=NMEA_Comma_Pos(p1,6,nRxLen);
	if(posx!=0XFF)
	{
		pst0183GpsData.cEastWest = *(p1+posx);
	}

#if 0
	//SOG, 最多只取小数点后一位
	posx=NMEA_Comma_Pos(p1,7,nRxLen);
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx,1,&bBlank);
		if(bBlank)
		{
			pst0183GpsData.speed=0xFFFF;
		}
		else
		{
			if(dx<1)
			{
				temp=temp*NMEA_Pow(10,1);
			}
			pst0183GpsData.speed=temp;
		}
	}

	//COG, 最多只取小数点后一位
	posx=NMEA_Comma_Pos(p1,8,nRxLen);
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx,1,&bBlank);
		if(bBlank)
		{
			pst0183GpsData.cog=0xFFFF;
		}
		else
		{
			if(dx<1)
			{
				temp=temp*NMEA_Pow(10,1);
			}
			pst0183GpsData.cog=temp;
		}
	}
#endif
	//UTC日期
	posx=NMEA_Comma_Pos(p1,9,nRxLen);
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p1+posx,&dx,0,&bBlank);
		pst0183GpsData.Day=temp/10000;
		pst0183GpsData.Month=(temp/100)%100;
		pst0183GpsData.Year=2000+temp%100;
	}

#if 0
	//角度磁差
	posx=NMEA_Comma_Pos(p1,10,nRxLen);
	if(posx!=0XFF)
	{
		temp = NMEA_Str2num(p1+posx,&dx,1,&bBlank);

		if(bBlank)
		{
			pst0183GpsData.mv=0xFFFF;
		}
		else
		{
			if(dx<1)
			{
				temp=temp*NMEA_Pow(10,1);
			}
			pst0183GpsData.mv=temp;
		}
	}

	//角度磁差方位
	posx=NMEA_Comma_Pos(p1,11,nRxLen);
	if(posx!=0XFF)
	{
		pst0183GpsData.mvEW = *(p1 + posx);
	}


	//定位模式
	posx=NMEA_Comma_Pos(p1,12,nRxLen);
	if(posx!=0XFF)
	{
		cPosMode=*(p1+posx);

		if(cPosMode=='N')
		{
			pst0183GpsData.quality=0;
		}
		else if(cPosMode=='D')
		{
			pst0183GpsData.quality=2;
		}
		else if(cPosMode=='P')
		{
			pst0183GpsData.quality=3;
		}
		else if(cPosMode=='A')
		{
			pst0183GpsData.quality=1;
		}
		else if(cPosMode=='R')
		{
			pst0183GpsData.quality=4;
		}
		else if(cPosMode=='F')
		{
			pst0183GpsData.quality=5;
		}
		else if(cPosMode=='E')
		{
			pst0183GpsData.quality=6;
		}
		else if(cPosMode=='M')
		{
			pst0183GpsData.quality=7;
		}
		else if(cPosMode=='S')
		{
			pst0183GpsData.quality=8;
		}
		else
		{
			pst0183GpsData.quality=0;
		}
	}

	if( (pst0183GpsData.quality > 0) && (pst0183GpsData.quality <= 6))
	{
		nGpsDataTimeCnt = 0;
		if( !bInternalModule )
		{
			bExRMCFix = TRUE;
			stLocal.cLatitudeDegree=pst0183GpsData.cLatitudeDegree;
			stLocal.lLatitudeMin=pst0183GpsData.lLatitudeMin/100;
			stLocal.cSouthNorth=pst0183GpsData.cSouthNorth;

			stLocal.cLongitudeDegree=pst0183GpsData.cLongitudeDegree;
			stLocal.lLongitudeMin=pst0183GpsData.lLongitudeMin/100;
			stLocal.cEastWest=pst0183GpsData.cEastWest;

			stLocal=CalculateLocalPositionToRef(stLocal,st0183DatumInfo.stPositionDeltaData);
			pst0183GpsData.cLatitudeDegree=stLocal.cLatitudeDegree;
			pst0183GpsData.lLatitudeMin=stLocal.lLatitudeMin*100;
			pst0183GpsData.cSouthNorth=stLocal.cSouthNorth;
			pst0183GpsData.cLongitudeDegree=stLocal.cLongitudeDegree;
			pst0183GpsData.lLongitudeMin=stLocal.lLongitudeMin*100;
			pst0183GpsData.cEastWest=stLocal.cEastWest;
		}
	}
	else
	{
		if( !bInternalModule )
		{
			bExRMCFix = FALSE;
		}
	}


	//Nav status
	posx=NMEA_Comma_Pos(p1,13,nRxLen);
	if(posx!=0XFF)
	{
		if(*(p1+posx)!=',')
		{
			if(*(p1+posx)=='S')
			{
				pst0183GpsData.Integrity=1;
			}
			else if(*(p1+posx)=='C')
			{
				pst0183GpsData.Integrity=2;
			}
			else if(*(p1+posx)=='U')
			{
				pst0183GpsData.Integrity=3;
			}
			else
			{
				pst0183GpsData.Integrity=0;
			}
		}
		else
		{
			pst0183GpsData.Integrity=0;
		}
	}
#endif
}


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

	//设置串口数据�?
	switch(nBits)
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |=CS8;
			break;
	}
	//设置奇偶校验�?
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


int uart_rx(void)
{
	int ret=-1;
	u8 rx_buf[UART_DATA_BUFF_LEN];
	char *temp;


	memset(rx_buf,0,UART_DATA_BUFF_LEN);

	int timeout=100;

	while(1){
		ret=read(fd,rx_buf,UART_DATA_BUFF_LEN);
		printf("rx_buf %s \n",rx_buf);
		if(ret>0){
			//printf("%s\n",rx_buf);
			temp=strtok(rx_buf,"\r\n");
			while(temp){
				if(find_nmea0183_head(temp, strlen(temp))==E_NMEA0183_RMC){
					api_nmea0183_rmc_analysis(temp,TRUE,strlen(temp));
					if(pst0183GpsData.quality!=1){
						save_log("gps can not locate\n");
						return -1;
					}
					return 0;
				}
				temp=strtok(NULL,"\r\n");
			}
		}
		//printf("timeout %d\n",timeout);
		timeout+=100;
		if(timeout==2000)
			break;
		usleep(100*1000);
	}

	save_log("get gps data timeout\n");

	return -1;

}


void Stop(int signo)
{
	printf("close fd\n");
    close(fd);
    _exit(0);
}

#if 1
void flush_gps_data(void)
{
		tcflush(fd, TCIFLUSH);		//������뻺��
		tcflush(fd, TCOFLUSH);		//����������
		tcflush(fd, TCIOFLUSH); 	//��������������
		ioctl(fd, TCFLSH, 0); 	 //������뻺��
		ioctl(fd, TCFLSH, 1); 	 //����������
		ioctl(fd, TCFLSH, 2); 	 //��������������
    	tcflush(fd, TCIOFLUSH);
}
#endif

void main(void)
{
	int i=0;
	int rx_status;
	char lat[20];
	char lng[20];

	fd = open(DEV_NAME, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(fd < 0) {
		save_log("\n");
		save_log("gps uart open failed\n");
		printf("GPS UART FAILED\n");
		return;
	}
	SetOpt(fd, 9600, 8, 'N', 1);

	rx_status = uart_rx();

	if(rx_status!=0){
		printf("GPS LOCATE FAILED\n");
	}else{
		printf("GPS LOCATE SUCC\n");
		snprintf(lat,20,"%d°%.3f'%c",pst0183GpsData.cLatitudeDegree,(double)pst0183GpsData.lLatitudeMin/100000,pst0183GpsData.cSouthNorth);
		snprintf(lng,20,"%d°%.3f'%c",pst0183GpsData.cLongitudeDegree,(double)pst0183GpsData.lLongitudeMin/100000,pst0183GpsData.cEastWest);
		save_log("lat:%s  lng:%s\n",lat,lng);
	}
	flush_gps_data();
	close(fd);
	return;
}


