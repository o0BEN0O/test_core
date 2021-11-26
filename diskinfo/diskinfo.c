#include <sys/statfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int get_system_tf_free(double *free)
{
	if(free == NULL)
		return -1;
	struct statfs diskInfo;
	statfs("/raymarine/Data",&diskInfo);
	unsigned long long totalBlocks = diskInfo.f_bsize;
	unsigned long long freeDisk = diskInfo.f_bfree*totalBlocks;
	unsigned long long availdisk=diskInfo.f_bavail*totalBlocks;
	printf("totalblocks %ld freedisk %ld availdisk %ld\n",totalBlocks,freeDisk,availdisk);

	*free = freeDisk;
	return 0;
}

void main (void)
{
	double free=0;
	get_system_tf_free(&free);
	return;
}
