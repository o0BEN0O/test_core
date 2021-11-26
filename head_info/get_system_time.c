#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#define unit 1024 /*unit KiB*/

#define version_pos 4096*unit
#define version_size 1*unit

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
	uint64_t sec=tv.tv_sec;
	uint64_t min=tv.tv_sec/60;
	struct tm cur_tm;//保存转换后的时间结果
	localtime_r((time_t*)&sec,&cur_tm);
	snprintf(cur_time,20,"%d-%02d-%02d %02d:00",cur_tm.tm_year+1900,cur_tm.tm_mon+1,cur_tm.tm_mday,cur_tm.tm_hour);
	//printf("current time is %s\n",cur_time);//打印当前时间
}


int main(int argc, char **argv)
{
	char sys_time[20];
	get_system_time(sys_time);
	printf("%s\n",sys_time);

}
