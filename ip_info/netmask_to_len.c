#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <errno.h>

int mask2len(unsigned int mask)
{
    /*eg: 255.255.255.0   255.0.255.255.0*/
    int bit=0,len=0;

    while( ~mask & (1ULL<<bit))
        bit++;
    len = 32 - bit;

    if(~mask >> bit){
        printf("mask is not standerd, get mask length error\n");
        return -1;
    }
    printf("mask:%8.8x    len=%d\n", mask, len);
    return len;
}


void main(int argc, char *argv[])
{
	unsigned int netmask_int;
	char netmask[17];
	int part1,part2,part3,part4;
	sscanf(argv[1],"%s",netmask);
	sscanf(netmask,"%d.%d.%d.%d",&part1,&part2,&part3,&part4);
	netmask_int=part1<<24|part2<<16|part3<<8|part4;
	int len;
	len=mask2len(netmask_int);
	return;
}

