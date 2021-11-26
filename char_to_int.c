#include <stdio.h>
#include <stdlib.h> 

void main(void)
{
	char buf[4]="6400";
	char buf1[4]={0x02,0x01,0x03,0x04};
	int temp;
	char *buf2;
	sscanf(buf,"%d",&temp);
	temp = temp/100;
	printf("%d\n",temp);
	sscanf(buf1,"%s",buf2);
	printf("%s\n",buf2);
}
