#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h> 

#define unit 1024 /*unit KiB*/


#define head_size 2*unit
#define head_pos 16*unit

#define version_pos 4096*unit
#define version_size 1*unit

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

void read_info_from_emmc(_version_info *version_info)
{
	FILE *fp=NULL;
	fp = fopen("/home/ben/ben/4g_router/head_info/version","rb");
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

int main(void)
{
	_version_info version_info;
	read_info_from_emmc(&version_info);
	printf("%s %s %s %s \n",version_info.uboot_version,version_info.kernel_version,version_info.system_version,version_info.total_version);
}

