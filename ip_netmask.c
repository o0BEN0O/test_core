#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>




void range_start_ip_calc(const char *ip,const char *netmask,char *start_ip)
{
	unsigned long ipv4=0,ipv4msk=0;
	unsigned long ipv4subnet=0;
	unsigned long start_ip_int=0;
	unsigned long tmp=0;
	struct sockaddr_in addr;

	ipv4 = inet_addr(ip);
	ipv4msk = inet_addr(netmask);

	ipv4subnet = ipv4&ipv4msk;
	//printf("%lx\n",ipv4subnet);
	tmp=(ipv4subnet>>24)+1;
	//printf("%lx\n",tmp);
	start_ip_int = (tmp<<24)|(ipv4subnet&0xffffff);
	//printf("%lx\n",start_ip_int);
	addr.sin_addr.s_addr=start_ip_int;

	strncpy(start_ip,inet_ntoa(addr.sin_addr),strlen(inet_ntoa(addr.sin_addr)));

	printf("start_ip %s\n",start_ip);
}

void range_end_ip_calc(const char *ip,const char *netmask,char *end_ip)
{
	unsigned long ipv4=0,ipv4msk=0;
	unsigned long ipv4subnet=0;
	unsigned long end_ip_int=0;
	unsigned long tmp=0;
	struct sockaddr_in addr;

	ipv4 = inet_addr(ip);
	ipv4msk = inet_addr(netmask);

	ipv4subnet = ipv4&ipv4msk;

	end_ip_int = (ipv4subnet|(~ipv4msk))&0x00000000ffffffff;
	tmp=(end_ip_int>>24)-1;
	end_ip_int = (tmp<<24)|(end_ip_int&0xffffff);
	addr.sin_addr.s_addr=end_ip_int;

	strncpy(end_ip,inet_ntoa(addr.sin_addr),strlen(inet_ntoa(addr.sin_addr)));

	printf("end_ip %s\n",end_ip);
}

int ip_range_check(const char *ip,const char *netmask,char *start_ip,char *end_ip)
//void ip_range_check(const char *ip,const char *netmask)
{
	char range_start_ip[32]="\0";
	char range_end_ip[32]="\0";
	unsigned long int_range_start_ip;
	unsigned long int_range_end_ip;
	unsigned long int_start_ip;
	unsigned long int_end_ip;
	unsigned long part1,part2,part3,part4;

	start_ip_calc(ip,netmask,range_start_ip);
	end_ip_calc(ip,netmask,range_end_ip);

	sscanf(range_start_ip,"%ld.%ld.%ld.%ld",&part1,&part2,&part3,&part4);
	int_range_start_ip=(part1<<24)|(part2<<16)|(part3<<8)|(part4);
	sscanf(range_end_ip,"%ld.%ld.%ld.%ld",&part1,&part2,&part3,&part4);
	int_range_end_ip = (part1<<24)|(part2<<16)|(part3<<8)|(part4);

	sscanf(start_ip,"%ld.%ld.%ld.%ld",&part1,&part2,&part3,&part4);
	int_start_ip = (part1<<24)|(part2<<16)|(part3<<8)|(part4);

	sscanf(end_ip,"%ld.%ld.%ld.%ld",&part1,&part2,&part3,&part4);
	int_end_ip = (part1<<24)|(part2<<16)|(part3<<8)|(part4);

	if(int_start_ip>int_end_ip){
		printf("1.%s %s\n",start_ip,end_ip);
		return -1;
	}else if(int_start_ip<int_range_start_ip){
		printf("2.%s %s\n",start_ip,range_start_ip);
		return -1;
	}else if(int_end_ip>int_range_end_ip){
		printf("3.%s %s\n",end_ip,range_end_ip);
		return -1;
	}
	return 0;
	//sscanf(range_end_ip,"%d.%d.%d.%d",int_range_start_ip[0],int_range_start_ip[1],int_range_start_ip[2],int_range_start_ip[3]);
}



void main(int argc, char **argv)
{
	char ip[32];
	char netmask[32];
	char subnet[32];
	char start_ip[32];
	char end_ip[32];

	int ret;

	strcpy(ip,argv[1]);
	strcpy(netmask,argv[2]);

		strcpy(start_ip,argv[3]);
			strcpy(end_ip,argv[4]);

	//start_ip_calc(ip,netmask,subnet);
	//end_ip_calc(ip,netmask,subnet);

	ret=ip_range_check(ip,netmask,start_ip,end_ip);
	
}

