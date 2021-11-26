#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define unit 1024 /*unit KiB*/
#define runtime_pos 4101*unit
#define runtime_size 16

#define dev_mmcblk "/dev/mmcblk2"

int main(void)
{
	int fd;
	FILE *fp_mmcblk=NULL;

	char empty_buf[runtime_size];

	memset(empty_buf,0,runtime_size);

	fp_mmcblk = fopen(dev_mmcblk,"rb+");
	if(fp_mmcblk == NULL){
		printf("open failed\n");
		return -1;
	}

/*clear bundle version*/
	fseek(fp_mmcblk,runtime_pos,SEEK_SET);
	fwrite(empty_buf,1,runtime_size,fp_mmcblk);
/*clear bundle version*/

	fflush(fp_mmcblk); //write 	cache

	fd = fileno(fp_mmcblk);  //get file description
	if(fd != -1)
	{
		fsync(fd);  //write to emmc from cache
    }

	fclose(fp_mmcblk);
	return 0;
}
