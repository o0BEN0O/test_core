#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void date_cpy(unsigned int *a,unsigned int *b)
{
	memcpy(b,a+1,4*(sizeof(unsigned int)));
}

unsigned char cal_stm8_upgrade_package_checksum(unsigned char *pData,unsigned short nDataLen)
{
	unsigned short i;
	unsigned char u8Data;

	u8Data=0;
	for(i=0;i<nDataLen;i++)
	{
		u8Data^=pData[i];
	}
	return u8Data;
	
}

void main(void)
{
	int j=0;
	unsigned long addr=0x00009080;
	unsigned char package_head[5];
	unsigned int a[5]={0x01,0x02,0x03,0x04,0x05};
	unsigned int b[4];
	for(j=0;j<5;j++){
		if(j<4)
			package_head[j]=(addr>>(8*(3-j))&0xff);
		else
			package_head[j]=cal_stm8_upgrade_package_checksum(package_head,4);
		printf("%x ",package_head[j]);
	}
	printf("\n");
	//date_cpy(a,b);
	//memcpy(b,a+1,4*(sizeof(unsigned int)));
	//printf("%x %x %x %x\n",b[0],b[1],b[2],b[3]);
	
}
