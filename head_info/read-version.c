#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define unit 1024 /*unit KiB*/

#define version_pos 4096*unit
#define version_size 1*unit

typedef struct {
	char uboot_version[16];
	char kernel_version[16];
	char system_version[16];
	char total_version[20];
}_version_info;

void read_info_from_emmc(_version_info *version_info)
{
	//char version_buf[version_size];
	char *version_buf;
	FILE *fp;
	
	fp = fopen("/dev/mmcblk2","rb+");
	if(fp == NULL){
		printf("open /dev/mmcblk2 failed\n");
		fclose(fp);
		return;
	}

	version_buf = (char*)malloc( sizeof(char) * (version_size));
	//version_info = (_version_info *)malloc(sizeof(char) * (version_size));
	fseek(fp,version_pos,SEEK_SET);
	fread(version_buf,version_size,1,fp);

	//version_info = (_version_info *) version_buf;

	memcpy(version_info,version_buf,version_size);
	printf("uboot version %s\n",version_info->uboot_version);
	printf("total version %s\n",version_info->total_version);
	
	fclose(fp);
	free(version_buf);
}

void main(void)
{
	_version_info version_info;
	//char path[32];

	//strcpy(path,argv[1]);
	
	read_info_from_emmc(&version_info);
	printf("uboot version %s\n",version_info.uboot_version);
	printf("total version %s\n",version_info.total_version);
	//free(version_info);

	return;
#if 0
	version_buf = (char*)malloc( sizeof(char) * (version_size));
	version_info = (_version_info *)malloc(sizeof(char) * (version_size));
	fseek(fp,version_pos,SEEK_SET);
	fread(version_buf,version_size,1,fp);

	//version_info = (_version_info *) version_buf;
	memcpy(version_info,version_buf,version_size);

	//printf("uboot version %s\n",version_info.uboot_version);
	//printf("total version %s\n",version_info.total_version);
	printf("uboot version %s\n",version_info->uboot_version);
	printf("total version %s\n",version_info->total_version);
	fclose(fp);
	free(version_buf);
	//printf("uboot_version %s   kernel_version %s  system_version %s  \n",version_info->uboot_version,version_info->kernel_version,version_info->system_version);
	//printf("uboot version %s\n",version_info.uboot_version);
	//printf("total version %s\n",version_info.total_version);
	printf("uboot version %s\n",version_info->uboot_version);
	printf("total version %s\n",version_info->total_version);
#endif

	//return 0;
}

