//使用strtok()函数:
 
#include <sys/stat.h>
#include<stdio.h>
#include<string.h>
int main(void)
{
	struct stat uboot_flag;
	int uboot_flag_size;
	int i=1;

	while( i < 120){
		usleep(500*1000);
		stat("/tmp/uboot_flag",&uboot_flag);
		perror("");
		uboot_flag_size = uboot_flag.st_size;
		printf("size %d time %d\n",uboot_flag_size,i);
		i++;
	}

	//printf("size %d time %d\n",uboot_flag_size,i);
}
