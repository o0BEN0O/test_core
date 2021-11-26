#include <sys/ioctl.h>

#include <net/if.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <string.h>

#include <stdio.h>



void get_local_mac(char *mac_addr)

{
	int sock_mac; 
	struct ifreq ifr_mac; 
	sock_mac = socket( AF_INET, SOCK_STREAM, 0 );
	if( sock_mac == -1)  
    {  
        perror("create socket failed...mac\n");  
        return; 
    }

	memset(&ifr_mac,0,sizeof(ifr_mac));     
    strncpy(ifr_mac.ifr_name, "eth0", sizeof(ifr_mac.ifr_name)-1);
	if( (ioctl( sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)  
	{  
		printf("mac ioctl error\n");  
		return ;  
	}

	sprintf(mac_addr,"%02x:%02x:%02x:%02x:%02x:%02x",  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);       
    
    close(sock_mac);  
    return; 
}

int main()

{

        char mac[64];

        memset(mac, 0, sizeof(mac));

        get_local_mac(mac);

		printf("mac %s\n",mac);

        return 0;

}
