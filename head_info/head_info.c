#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>


#define unit 1024 /*unit KiB*/

#define head_size 2*unit
#define head_pos 16*unit

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

unsigned short Crc16CCITCalculate(unsigned char const *pData,unsigned long lDataLength)
{
        unsigned short ushort;
        unsigned short crc = 0;
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

void get_system_time(char *cur_time)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);//获取1970-1-1到现在的时间结果保存到tv中
	uint64_t sec=tv.tv_sec;
	uint64_t min=tv.tv_sec/60;
	struct tm cur_tm;//保存转换后的时间结果
	localtime_r((time_t*)&sec,&cur_tm);
	snprintf(cur_time,20,"%d-%02d-%02d %02d:00",cur_tm.tm_year+1900,cur_tm.tm_mon+1,cur_tm.tm_mday,cur_tm.tm_hour);
	//printf("current time is %s\n",cur_time);//打印当前时间
}


typedef struct {
	char uboot_version[16];
	char kernel_version[16];
	char system_version[16];
	unsigned long uboot_pos;
	unsigned long kernel_pos;
	unsigned long system_pos;
	unsigned long uboot_size;
	unsigned long kernel_size;
	unsigned long system_size;
	unsigned short checksum;
	bool force_upgrade;
	char total_version[20];
}_head_info;

char *info_buf;

int main(int argc, char **argv)
{
	int file_size;
	struct stat file;
	FILE *fp;
	FILE *fp_txt;
	char *data_buf;
	_head_info head_info;
	_head_info *read_info;
	unsigned long data_size;
	unsigned long data_start_pos;
	char path[1024];
	head_info.force_upgrade = false;

	info_buf = (char*)malloc( sizeof(char) * (head_size) );

	get_system_time(head_info.total_version);

	strcpy(head_info.uboot_version,argv[1]);
	head_info.uboot_pos = strtoul(argv[2],NULL,0)*unit;
	head_info.uboot_size = strtoul(argv[3],NULL,0)*unit;

	strcpy(head_info.kernel_version,argv[4]);
	head_info.kernel_pos = strtoul(argv[5],NULL,0)*unit;
	head_info.kernel_size = strtoul(argv[6],NULL,0)*unit;

	strcpy(head_info.system_version,argv[7]);
	head_info.system_pos = strtoul(argv[8],NULL,0)*unit;
	head_info.system_size = strtoul(argv[9],NULL,0)*unit;

	head_info.force_upgrade = strtoul(argv[10],NULL,0);

	strcpy(path,argv[11]);

	printf("path %s\n",path);

	stat(path,&file);
	file_size = file.st_size;

	data_start_pos = head_info.uboot_pos;

	data_size = file_size - data_start_pos;

	data_buf = (char*)malloc( sizeof(char) * (data_size+1) );

	fp = fopen(path,"rb+");
	if(fp == NULL){
		printf("open %s failed\n",path);
		fclose(fp);
		free(info_buf);
		free(data_buf);
		return -1;
	}

	fseek(fp,data_start_pos,SEEK_SET);
	fread(data_buf,1,data_size,fp);

	head_info.checksum = Crc16CCITCalculate(data_buf,data_size);

	printf("head_info.checksum = 0x%04x\n",head_info.checksum);


	memcpy(info_buf,&head_info,sizeof(head_info));
	read_info = (_head_info *) info_buf;
	printf("%s\n",read_info->uboot_version);
	printf("checksum 0x%04x\n",read_info->checksum);
	fseek(fp,head_pos,SEEK_SET);
	fwrite(info_buf,head_size,1,fp);

	fclose(fp);
	free(info_buf);
	free(data_buf);

	return 0;

}
