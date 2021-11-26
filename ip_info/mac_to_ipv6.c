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

void mac_to_ipv6addr(const char *mac)
{
	char ipv6_addr[29];
	char ipv6_prefix[6]="fe80::";
	int part1,part2,part3,part4,part5,part6;
	unsigned short ipv6_part1,ipv6_part2,ipv6_part3,ipv6_part4;
	memset(ipv6_addr,0,29);
	unsigned short ff=0xff;
	unsigned short fe=0xfe;
	sscanf(mac,"%x:%x:%x:%x:%x:%x",&part1,&part2,&part3,&part4,&part5,&part6);//00:02:00:04:00:b3
	part1=part1^0x02;
	ipv6_part1=(part1<<8|part2);
	ipv6_part2=(part3<<8|ff);
	ipv6_part3=(fe<<8|part4);
	ipv6_part4=(part5<<8|part6);

	snprintf(ipv6_addr,29,"%s%x:%x:%x:%x",ipv6_prefix,ipv6_part1,ipv6_part2,ipv6_part3,ipv6_part4);
	printf("ipv6_addr %s\n",ipv6_addr);
}

void main(int argc, char *argv[])
{
	char mac[17];
	sscanf(argv[1],"%s",mac);
	mac_to_ipv6addr(mac);
}

