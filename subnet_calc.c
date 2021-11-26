#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>


#define max_lease_time 43200;

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
	ip_range_end = (subnet_int|(~netmask_int))&0x00000000ffffffff;
	printf("%lx  %lx  %lx\n",ip_start_int&netmask_int,subnet_int,(subnet_int|(~netmask_int))&0x00000000ffffffff);
	
	if((ip_start_int&netmask_int) != subnet_int || ip_start_int == subnet_int || ip_start_int == ip_range_end){
		printf("start ip address over range\n");
		return -1;
	}

	ip_end_int = inet_addr(ip_addr_end);
	printf("%lx\n",ip_end_int);
	if((ip_end_int&netmask_int) != subnet_int || ip_end_int == subnet_int || ip_end_int == ip_range_end){
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


int test(void)
{
	char ipv4Netsub[64] = {0};
    char ipv4String[64]={0};
    char ipv4Msk[64]={0};
    char ipv4StringTmp[64]={0};
    unsigned long ipv4=0,ipv4msk=0;
    unsigned long ipv4Start=0,ipv4End=0;
    struct sockaddr_in   add;

    //strcpy(ipv4String,"192.168.1.230");
    //strcpy(ipv4Msk,"255.248.0.0");

    //strcpy(ipv4String,"10.0.0.0");
    //strcpy(ipv4Msk,"255.248.0.0");
    strcpy(ipv4String,"198.18.248.1");
    strcpy(ipv4Msk,"255.255.255.0");

    strcpy(ipv4StringTmp,ipv4String);
    if(true == IsIpv4(ipv4StringTmp))
    {
        printf("%s is ipv4\n",ipv4String);
    }else{
        printf("%s is not ipv4\n",ipv4String);
        return -1;
    }
    ipv4=inet_addr(ipv4String);
    ipv4msk=inet_addr(ipv4Msk);
	printf("ipvmas %lx\n",ipv4msk);
	if (((ipv4msk-1) | ipv4msk) != 0xffffffff){
		printf("illega netmask\n");
	}
    ipv4Start=ipv4&ipv4msk;
    add.sin_addr.s_addr=ipv4Start;
	strncpy(ipv4Netsub,inet_ntoa(add.sin_addr),strlen(inet_ntoa(add.sin_addr)));
    printf("ipv4 start %s+1\n",ipv4Netsub);
    ipv4End=ipv4|(~ipv4msk);
	//printf("%lx\n",ipv4End);
    add.sin_addr.s_addr=ipv4End;
    printf("ipv4 End %s-1\n",inet_ntoa(add.sin_addr));
	ipv4=inet_addr(inet_ntoa(add.sin_addr));
	ipaddr_range_check(ipv4Netsub,ipv4Msk,"198.18.248.1","198.18.255.255");
    return 0;
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

int dhcp_server_setting(const char *ip,const char *netmask,const char *start_ip,const char *end_ip,int lease_time)
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

	if(ipaddr_range_check(subnet,netmask,start_ip,end_ip) < 0){
		printf("error ipaddr ip range\n");
		return -1;
	}

	if(lease_time > max_lease_time){
		printf("lease_time is over %d\n",max_lease_time);
		return -1;
	}

	return 0;
}

void main()
{
	int ret;
	//test();
	ret = dhcp_server_setting("198.18.248.1","255.255.248.0","198.18.248.2","198.18.255.254",21800);
}

