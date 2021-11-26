#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h> 
#include <time.h>
#include <sys/time.h>

#define unit 1024 /*unit KiB*/

#define unit 1024 /*unit KiB*/

#define head_size 2*unit
#define head_pos 16*unit

#define version_pos 4096*unit
#define version_size 1*unit

#define dev_mmcblk "/dev/mmcblk2"

typedef struct {
	char uboot_version[16];
	char kernel_version[16];
	char system_version[16];
	char total_version[20];
	unsigned long uboot_pos;
	unsigned long kernel_pos;
	unsigned long system_pos;
	unsigned long uboot_size;
	unsigned long kernel_size;
	unsigned long system_size;
	unsigned short checksum;
	bool force_upgrade;
}_head_info;

typedef struct {
	char uboot_version[16];
	char kernel_version[16];
	char system_version[16];
	char total_version[20];
}_version_info;

void get_system_time(char *cur_time)
{
        struct timeval tv;
        gettimeofday(&tv,NULL);//获取1970-1-1到现在的时间结果保存到tv中
        unsigned long sec=tv.tv_sec;
        unsigned long min=tv.tv_sec/60;
        struct tm cur_tm;//保存转换后的时间结果
        localtime_r((time_t*)&sec,&cur_tm);
        snprintf(cur_time,20,"%d-%02d-%02d %02d:00",cur_tm.tm_year+1900,cur_tm.tm_mon+1,cur_tm.tm_mday,cur_tm.tm_hour);
        //printf("current time is %s\n",cur_time);//打印当前时间
}

void read_info_from_emmc(_version_info *version_info)
{
	FILE *fp=NULL;
	fp = fopen(dev_mmcblk,"rb");
	if(fp == NULL){
		printf("open failed\n");
		fclose(fp);
		return;
	}

	fseek(fp,version_pos,SEEK_SET);
	fread(version_info,1,sizeof(_version_info),fp);
	printf("u%s k%s s%s tt%s \n",version_info->uboot_version,version_info->kernel_version,version_info->system_version,version_info->total_version);
	fclose(fp);
}

int upgrade_version_info(_version_info *version)
{
	FILE *fp_mmcblk=NULL;
	char *version_buf;
	//_head_info head_info;
	//_version_info version;
	//memset(version.uboot_version,"\0",16);
	//memset(version.kernel_version,"\0",16);
	//memset(version.system_version,"\0",16);
	//memset(version.total_version,"\0",20);

	//fseek(fp,head_pos,SEEK_SET);
	//fread(&head_info,sizeof(_head_info),1,fp);

	fp_mmcblk = fopen(dev_mmcblk,"rb+");
	if(fp_mmcblk == NULL){
		printf("open failed\n");
		fclose(fp_mmcblk);
		return -1;
	}
	//strncpy(version.total_version, head_info.total_version,strlen(head_info.total_version));
	//strncpy(version.uboot_version, head_info.uboot_version,strlen(head_info.uboot_version));
	//strncpy(version.kernel_version, head_info.kernel_version,strlen(head_info.kernel_version));
	//strncpy(version.system_version, head_info.system_version,strlen(head_info.system_version));
	//printf("strlen tt %d\n",strlen(head_info.total_version));
	printf("u%s k%s s%s tt%s \n",version->uboot_version,version->kernel_version,version->system_version,version->total_version);
	printf("\n");
	version_buf = (char*)malloc( sizeof(char) * (version_size));
	memset(version_buf,0,version_size);
	memcpy(version_buf,version,sizeof(_version_info));

	fseek(fp_mmcblk,version_pos,SEEK_SET);
	fwrite(version_buf,1,version_size,fp_mmcblk);

	fclose(fp_mmcblk);
	free(version_buf);
	return 0;
}

int main(int argc, char **argv)
{
	//FILE *fp=NULL;
	_version_info version_info;
	memset(&version_info,0,sizeof(_version_info));
	get_system_time(version_info.total_version);
	strncpy(version_info.uboot_version,"V0.02",strlen("V0.02"));
	strncpy(version_info.kernel_version,"V0.06",strlen("V0.06"));
	strncpy(version_info.system_version,"V0.38",strlen("V0.38"));
	//version_info.uboot_version="V0.02";
	//version_info.uboot_version="V0.03";
	//version_info.uboot_version="V0.28";
	//fp = fopen("/home/ben/ben/4g_router/head_info/lin.upgrade","rb+");
	upgrade_version_info(&version_info);
	//fclose(fp);
	memset(&version_info,0,sizeof(_version_info));
	read_info_from_emmc(&version_info);
	printf("u%s k%s s%s tt%s \n",version_info.uboot_version,version_info.kernel_version,version_info.system_version,version_info.total_version);

}

