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

#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>

//#define IOCTRL_IPV4_ADDR	SIOCGIFADDR
//#define IOCTRL_NETMASK		SIOCGIFNETMASK
//#define IOCTRL_HWADDR		SIOCGIFHWADDR
//#define IOCTRL_IPV6_ADDR	SIOCGIFHWADDR

typedef enum{
	IOCTRL_IPV4_ADDR=SIOCGIFADDR,
	IOCTRL_NETMASK=SIOCGIFNETMASK,
	IOCTRL_HWADDR=SIOCGIFHWADDR,
	IOCTRL_IPV6_ADDR=SIOCGIFHWADDR+1,
}EM_NETDEV_IOCTRL;

void mac_to_ipv6addr(const char *mac, char *ipv6_addr)
{
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
	//printf("ipv6_addr %s\n",ipv6_addr);
}

void get_netdev_info(const        char *netdev, char *netdev_info,EM_NETDEV_IOCTRL netdev_io_ctrl,int netdev_info_len)
{
	struct ifreq ifr;
	int sock;
	struct sockaddr_in *sock_in;
	memset(netdev_info,0,netdev_info_len);
	//printf("sizeof %d\n",sizeof(netdev_info));
	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if( sock == -1)
    {
        perror("create socket failed...mac\n");
        return;
    }

	memset(&ifr,0,sizeof(struct ifreq));

    strncpy(ifr.ifr_name, netdev, sizeof(ifr.ifr_name)-1);

	if(netdev_io_ctrl==IOCTRL_IPV6_ADDR){
		if( (ioctl( sock, netdev_io_ctrl-1, &ifr)) < 0)
		{
			printf(" ioctl error\n");
			goto out;
		}
	}else{
		if( (ioctl( sock, netdev_io_ctrl, &ifr)) < 0)
		{
			printf(" ioctl error\n");
			goto out;
		}
	}

	switch(netdev_io_ctrl){
		case IOCTRL_IPV4_ADDR:
			sock_in=(struct sockaddr_in*)(&(ifr.ifr_addr));
			inet_ntop(AF_INET,&(sock_in->sin_addr), netdev_info, netdev_info_len);
		break;

		case IOCTRL_NETMASK:
			sock_in=(struct sockaddr_in*)(&(ifr.ifr_netmask));
			inet_ntop(AF_INET,&(sock_in->sin_addr), netdev_info, netdev_info_len);
		break;

		case IOCTRL_HWADDR:
		case IOCTRL_IPV6_ADDR:
			sock_in=(struct sockaddr_in*)(&(ifr.ifr_hwaddr));
			snprintf(netdev_info,netdev_info_len,"%02x:%02x:%02x:%02x:%02x:%02x",
						(unsigned char)ifr.ifr_hwaddr.sa_data[0],
						(unsigned char)ifr.ifr_hwaddr.sa_data[1],
						(unsigned char)ifr.ifr_hwaddr.sa_data[2],
						(unsigned char)ifr.ifr_hwaddr.sa_data[3],
						(unsigned char)ifr.ifr_hwaddr.sa_data[4],
						(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
			if(netdev_io_ctrl==IOCTRL_IPV6_ADDR)
			{
				char ipv6_addr[29];
				mac_to_ipv6addr(netdev_info,ipv6_addr);
				memset(netdev_info,0,netdev_info_len);
				memcpy(netdev_info,ipv6_addr,strlen(ipv6_addr));
			}
		break;
	}

out:
	close(sock);
    return;
}

void main(void)
{
	char netdev_info[29]="\0";
	get_netdev_info("wlan0",netdev_info,IOCTRL_HWADDR,sizeof(netdev_info));
	printf("%s\n",netdev_info);

}
