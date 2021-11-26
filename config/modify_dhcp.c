#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>



#define dhcp_conf "/home/ben/ben/4g_router/test/dhcpd.conf"
#define routers "option routers"
#define end ";"
#define router_addr "10.10."
#define space " "
 
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

int IsSubnetMask(char* subnet)
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

void main(int argc, char *argv[])
{
	struct stat file;
	int start;
	int len;
	int file_size;
	int fd=0;
	char *dhcp;
	char *point;
	char *end_point;
	char *buf;
	char *space_point;
	int space_cnt=0;
	int i=0;

	//CheckIPAddressFormat(router_addr);
	IsSubnetMask("255.255.248.0");
	fd = open(dhcp_conf,O_RDWR);
	if(fd < 0){
		perror("open");
		printf("open fail!");
		close(fd);
		return;
	}
	stat(dhcp_conf,&file);
	file_size = file.st_size;
	dhcp = (char *)malloc(file_size);
	read(fd,dhcp,file_size);
	point = strstr(dhcp,routers);
	start = (point-dhcp);
	end_point = strstr(point,end);
	len = (end_point-point);
	space_cnt = (len-strlen(routers)-strlen(router_addr)-strlen(end))+1;
	space_point = (char *)malloc(space_cnt);
	for(i=0;i<space_cnt;i++){
		strncpy(space_point+i,space,1);
	}
	printf("space_cnt %d\n",space_cnt);
	buf = (char *)malloc(strlen(routers)+strlen(router_addr)+strlen(end)+space_cnt);
	sprintf(buf,"%s%s%s%s",routers,space_point,router_addr,end);
	printf("buf %s\n",buf);
	lseek(fd,start,SEEK_SET);
	write(fd,buf,len+1);
	
	free(dhcp);
	free(buf);
	free(space_point);
	//free(buf);
	close(fd);
	return;
}

