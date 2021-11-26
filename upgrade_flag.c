#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <string.h>
#include<stdlib.h>

#define uboot_size "/tmp/uboot_size"
#define block_size 512

#define UBOOT_FLAG "/tmp/uboot_flag"

int upgrade_parm(char *file,unsigned long size)
{
    FILE *fp;

	size = size/block_size;
	char buf[8];
	//itoa(s,buf,10);
	sprintf(buf,"%ld",size);
    printf("s %s\n",buf);
    fp = fopen(file , "wt+" );
    fwrite(buf,strlen(buf),1,fp);
	fclose(fp);
   // perror("write");
    return 0;
}

int read_file(char *file,char *buf,int size)
{
    FILE *fp=NULL;
	int ret;

	//itoa(s,buf,10);
    fp = fopen(file , "rt" );
	if(fp==NULL){
		printf("open %s failed!\n",file);
		return -1;
	}

    ret = fread(buf,1,size,fp);
	printf("ret %d\n",ret);
	if(ret!=size){
		printf("read %s failed!\n",file);
		return -1;
	}
	fclose(fp);
   // perror("write");
    return 0;
}

int stringFind(const char *string, const char *dest) {
    if (string == NULL || dest == NULL) return -1;

    int i = 0;
    int j = 0;
    while (string[i] != '\0') {
        if (string[i] != dest[0]) {
            i ++;
            continue;
        }
        j = 0;
        while (string[i+j] != '\0' && dest[j] != '\0') {
            if (string[i+j] != dest[j]) {
                break;
            }
            j ++;
        }
        if (dest[j] == '\0') return i;
        i ++;
    }
    return -1;
}

dd if=/raymarine/Data/fsl-image-validation-imx-imx8mmevk-20200828023923.upgrade of=/dev/mmcblk2boot1 bs=512 count=4096 skip=66 seek=66 
int main(void)
{
	int ret;
	char *buf;
	struct stat uboot_flag;
	int uboot_flag_size = 0;
	stat(UBOOT_FLAG,&uboot_flag);
	uboot_flag_size = uboot_flag.st_size;
	buf = (char *)malloc(uboot_flag_size-1);
	memset(buf,0,uboot_flag_size-1);
	printf("buf %s %d\n",buf,strlen(buf));
	read_file(UBOOT_FLAG,buf,uboot_flag_size-1);
	printf("buf %s %d\n",buf,uboot_flag_size);
	ret = stringFind(buf,"failed");
	printf("ret %d\n",ret);
}
