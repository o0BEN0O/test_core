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



#define max_lease_time 43200

#define path_dhcp_conf "/home/ben/ben/4g_router/test/dhcpd.conf"

#define SUBNET 		"subnet"
#define NETMASK 	"netmask"
#define IP_RANGE 	"range dynamic-bootp"
#define DNS 		"option domain-name-servers"
#define LEASE_TIME 	"default-lease-time"

#define semicolon 	";"
#define left_braces	"{"
#define space		" "


typedef struct{
	char *options;
	char *end;
}_dhcp_conf;

_dhcp_conf dhcp_conf[]={
	{SUBNET,left_braces},
	{IP_RANGE,semicolon},
	{DNS,semicolon},
	{LEASE_TIME,semicolon},
};

bool IsIpv4(char*str)
{
    char* ptr;
    int count = 0;
    const char *p = str;

    //1、判断是不是三个 ‘.’
    //2、判断是不是先导0
    //3、判断是不是四部分数
    //4、第一个数不能为0

    while(*p !='\0')
    {
        if(*p == '.')
        count++;
        p++;
    }

    if(count != 3)
        return false;

    count = 0;
    ptr = strtok(str,".");
    while(ptr != NULL)
    {   
        count++;
        if(ptr[0] == '0' && isdigit(ptr[1]))
            return false;
        int a = atoi(ptr);
        if(count == 1 && a == 0)
            return false;
        if(a<0 || a>255)
            return false;
        ptr = strtok(NULL,".");

    }
    if(count == 4)
        return true;
    else
        return false;
}

int ipaddr_range_check(const char *subnet,const char *netmask,const char *ip_addr_start,const char *ip_addr_end)
{
	unsigned long subnet_int = 0;
	unsigned long netmask_int = 0;
	unsigned long ip_start_int = 0;
	unsigned long ip_end_int = 0;

	unsigned long ip_range_end=0;

	printf("%s %s\n",subnet,netmask);
	subnet_int = inet_addr(subnet);
	netmask_int = inet_addr(netmask);
	ip_start_int = inet_addr(ip_addr_start);
	ip_end_int = inet_addr(ip_addr_end);
	ip_range_end = (subnet_int|(~netmask_int))&0x00000000ffffffff;
	printf("%lx  %lx  %lx\n",ip_start_int&netmask_int,subnet_int,(subnet_int|(~netmask_int))&0x00000000ffffffff);

	if(netmask_int == 0xffffffff){
		if(subnet_int == ip_start_int && subnet_int == ip_end_int){
			printf("%s can not used in dhcpd server\n",netmask);
			return -2;
		}
	}
	
	if((ip_start_int&netmask_int) != subnet_int || ip_start_int < subnet_int || ip_start_int > ip_range_end){
		printf("start ip address over range\n");
		return -1;
	}

	//printf("%lx\n",ip_end_int);
	if((ip_end_int&netmask_int) != subnet_int || ip_end_int < subnet_int || ip_end_int > ip_range_end){
		printf("%lx %lx %lx\n",ip_end_int,subnet_int,ip_range_end);
		printf("end ip address over range\n");
		return -1;
	}

	if(ip_end_int < ip_start_int){
		printf("start ip address is over end ip address");
		return -1;
	}

	return 0;
}

int CheckIPAddressFormat(const char *strIPAddress)
{
    struct in_addr addr;
    int ret;
    volatile int local_errno;
 
    errno = 0;
    ret = inet_pton(AF_INET, strIPAddress, &addr);
    local_errno = errno;
    if (ret > 0)
        fprintf(stderr, "\"%s\" is a valid IPv4 address\n", strIPAddress);
    else if (ret < 0)
        fprintf(stderr, "EAFNOSUPPORT: %s\n", strerror(local_errno));
    else 
        fprintf(stderr, "\"%s\" is not a valid IPv4 address\n", strIPAddress);
 
    return ret;
}

int IsSubnetMask(const char* subnet)
{
    if(CheckIPAddressFormat (subnet))
    {
        unsigned int b = 0, i, n[4];
        sscanf(subnet, "%u.%u.%u.%u", &n[3], &n[2], &n[1], &n[0]);
        for(i = 0; i < 4; ++i) //将子网掩码存入32位无符号整型
            b += n[i] << (i * 8); 
        b = ~b + 1;
        if((b & (b - 1)) == 0){   //判断是否为2^n
			printf("well netmask\n");
			return 1;
        }
    }
    return -1;
}


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

	range_start_ip_calc(ip,netmask,range_start_ip);
	range_end_ip_calc(ip,netmask,range_end_ip);

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


void subnet_calc(const char *ip,const char *netmask,char *subnet)
{
	unsigned long ipv4=0,ipv4msk=0;
	unsigned long ipv4subnet=0;
	struct sockaddr_in addr;

	ipv4 = inet_addr(ip);
	ipv4msk = inet_addr(netmask);

	ipv4subnet = ipv4&ipv4msk;
	printf("%lx\n",ipv4subnet);
	addr.sin_addr.s_addr=ipv4subnet;

	strncpy(subnet,inet_ntoa(addr.sin_addr),strlen(inet_ntoa(addr.sin_addr)));

	//printf("subnet %s \n",subnet);
}

void write_dhcpd_conf(char *ip,char *subnet,char *netmask,char *start_ip,char *end_ip,int lease_time)
{
	char dhcpd_conf[1024]="\0";
	int fd=0;
	fd = open(path_dhcp_conf,O_WRONLY|O_TRUNC);
	if(fd < 0){
		perror("open");
		printf("open fail!");
		close(fd);
		return;
	}

	snprintf(dhcpd_conf,1024,
		"ddns-update-style interim;\n\
ignore client-updates;\n\
\n\
subnet %s netmask %s            {\n\
\n\
        option routers                               %s;\n\
        option subnet-mask                           %s;\n\
\n\
        option time-offset                           -18000; # Eastern Standard Time\n\
        range dynamic-bootp                          %s %s;\n\
        option domain-name                           \"imx8mmevk\";\n\
        option domain-name-servers                   8.8.8.8,8.8.4.4;;\n\
        default-lease-time                           %d;\n\
        max-lease-time                               43200;\n\
\n\
}\n",subnet,netmask,ip,netmask,start_ip,end_ip,lease_time);
	write(fd,dhcpd_conf,strlen(dhcpd_conf));
	close(fd);
}
#if 0
int dhcp_server_setting(const char *ip,
const char *netmask,
const char *start_ip,
const char *end_ip,
const char *main_dns,
const char*snd_dns,
int lease_time)
{
	char subnet[64] = {0};

	if(CheckIPAddressFormat(ip) <= 0){
		printf("error ip\n");
		return -1;
	}

	if(IsSubnetMask(netmask) < 0){
		printf("error netmask\n");
		return -1;
	}

	subnet_calc(ip,netmask,subnet);
	printf("subnet %s\n",subnet);

	if(CheckIPAddressFormat(start_ip) <= 0){
		printf("error start ip address\n");
		return -1;
	}

	if(CheckIPAddressFormat(end_ip) <= 0){
		printf("error end ip address\n");
		return -1;
	}

	if(ip_range_check(ip,netmask,start_ip,end_ip) < 0){
		printf("error ipaddr range\n");
		return -1;
	}

	if(lease_time > max_lease_time||lease_time<0){
		printf("lease_time is over %d\n",max_lease_time);
		return -1;
	}

	write_dhcpd_conf(ip,subnet,netmask,start_ip,end_ip,lease_time);

	return 0;
}
#endif
void main()
{
	int ret;
	//test();
	 write_dhcpd_conf("198.168.202.1","198.168.202.0","255.255.255.0","198.168.202.10","198.168.202.100",21800);
}

