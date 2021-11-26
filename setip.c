#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <error.h>
#include <net/route.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define UP IFF_UP|IFF_RUNNING
#define DOWN IFF_UP

#define N_CLR            0x01
#define M_CLR            0x02
#define N_SET            0x04
#define M_SET            0x08
#define N_ARG            0x10
#define M_ARG            0x20

#define SET_MASK         (N_SET | M_SET)
#define N_MASK           (N_CLR | N_SET | N_ARG)

int macAddrSet(unsigned int* mac)
{
	struct ifreq temp;
	struct sockaddr* addr;
 
	int fd = 0;
	int ret = -1;
	
	if((0 != getuid()) && (0 != geteuid()))
		return -1;
 
	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		return -1;
	}
 
	strcpy(temp.ifr_name, "eth0");
	addr = (struct sockaddr*)&temp.ifr_hwaddr;
	
	addr->sa_family = ARPHRD_ETHER;
	memcpy(addr->sa_data, mac,6);
	
		if( (ioctl(fd, SIOCSIFHWADDR, &temp)) < 0)
	{
		perror("set hw error:");
		close(fd);
		return;
	}
	
	close(fd);
	return ret;
}

int setif_up_down(char *ifname,int selector)
{
	int fd;
	unsigned char mask = N_MASK;;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
		perror("socket   error");
		close(fd);
		return -1;
    }
    memset(&ifr,0,sizeof(ifr));
    strcpy(ifr.ifr_name,ifname);
	ioctl(fd, SIOCGIFFLAGS, &ifr);
	if(selector == DOWN)
		mask &= N_CLR;
	if(selector == UP)
		mask &= N_SET;

	ifr.ifr_flags = selector;
	if (mask & SET_MASK)
		ifr.ifr_flags |= selector;
	else
		ifr.ifr_flags &= ~selector;
	if(ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
	{
		perror("SIOCSIFFLAGS error:");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

//int SetIfAddr(char *ifname, char *Ipaddr, char *mask,char *gateway)
int SetIfAddr(char *ifname, char *Ipaddr, char *mask)
{
    int fd;
    int rc;
    struct ifreq ifr;
    struct sockaddr_in *sin;
    struct rtentry  rt;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
            perror("socket   error");
            return -1;
    }
    memset(&ifr,0,sizeof(ifr));
    strcpy(ifr.ifr_name,ifname);
	
    sin = (struct sockaddr_in*)&ifr.ifr_addr; 
    sin->sin_family = AF_INET;
    //IP地址
    if(inet_aton(Ipaddr,&(sin->sin_addr)) < 0)   
    {     
        perror("inet_aton   error");     
        goto error;   
    }    
 
    if(ioctl(fd,SIOCSIFADDR,&ifr) < 0)   
    {     
        perror("ioctl   SIOCSIFADDR   error");     
        goto error;    
    }
    //子网掩码
    if(inet_aton(mask,&(sin->sin_addr)) < 0)   
    {     
        perror("inet_pton   error");     
        goto error;     
    }    
    if(ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
    {
		perror("ioctl");
		goto error;
    }
#if 0
    //网关
    memset(&rt, 0, sizeof(struct rtentry));
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    if(inet_aton(gateway, &sin->sin_addr)<0)
    {
       printf("inet_aton error\n");
	   goto error;
    }
    memcpy(&rt.rt_gateway,sin,sizeof(struct sockaddr_in));
    ((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;
    ((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;
    rt.rt_flags = RTF_GATEWAY;
    if (ioctl(fd, SIOCADDRT, &rt)<0)
    {
    	perror("SIOCADDRT:");
        printf("error in set router addr\n");
		goto error;
    }
#endif
    close(fd);
    return rc;
error:
	close(fd);
    return -1;
}

void set_local_mac(unsigned char *mac_addr)
{
	int sock_mac;
	struct ifreq ifr_mac;
	struct sockaddr* addr;
	int i;
	sock_mac = socket( AF_INET, SOCK_STREAM, 0 );
	if( sock_mac == -1)
    {
        perror("create socket failed...mac\n");
        return;
    }

	memset(&ifr_mac,0,sizeof(ifr_mac));
    strncpy(ifr_mac.ifr_name, "eth0", sizeof(ifr_mac.ifr_name)-1);
	addr = (struct sockaddr*)&ifr_mac.ifr_hwaddr;
	addr->sa_family = ARPHRD_ETHER;
	//memcpy(addr->sa_data,mac_addr,6);
	for(i=0;i<6;i++){
		//unsigned int a;
		memcpy(&addr->sa_data[i],mac_addr+i,1);
		//printf("%02x ",a);
	}
	printf("\n");
	printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
	addr->sa_data[0],addr->sa_data[1],addr->sa_data[2],addr->sa_data[3],addr->sa_data[4],addr->sa_data[5]);
	if( (ioctl( sock_mac, SIOCSIFHWADDR, &ifr_mac)) < 0)
	{
		perror("set hw error:");
		close(sock_mac);
		return;
	}

    close(sock_mac);
    return;
}

int mac_str_to_bin(char *str,unsigned char *mac)
{
	int i;
	char *s,*e;

	if((mac == NULL) || (str == NULL))
	{
		return -1;
	}

	s = (char *)str;
	for(i=0;i<6;++i){
		mac[i] = s?strtoul(s,&e,16):0;
		if(s)
			s = (*e)?e+1:e;
	}
	return 0;
}

void main(void)
{
	//setif_up_down("eth0",UP);
	//SetIfAddr("eth0","198.18.248.4","255.255.248.0","198.18.248.1");
	unsigned char n[6];
	//unsigned int mac[6]={0x40,0xfc,0xe9,0xdf,0xd8,0x45};
	char mac[20]="80:32:C0:AF:55:AB";
	mac_str_to_bin(mac,n);
	printf("%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n",n[0],n[1],n[2],n[3],n[4],n[5]);
	//sscanf(mac,"%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%[^:]",n);
	//printf("%s:%s:%s:%s:%s:%s\n",n[0],n[1],n[2],n[3],n[4],n[5]);
	//set_local_mac(mac);
	//macAddrSet(mac);
}

