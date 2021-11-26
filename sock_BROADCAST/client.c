#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

 
#define LEN 					256

char *get_local_ip(void)

{

        int fd, intrface, retn = 0;

		char *ip = NULL;

        struct ifreq buf[INET_ADDRSTRLEN];

        struct ifconf ifc;

        if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)

        {

                ifc.ifc_len = sizeof(buf);

                // caddr_t,linux内核源码里定义的：typedef void *caddr_t；

                ifc.ifc_buf = (caddr_t)buf;

                if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))

                {

                        intrface = ifc.ifc_len/sizeof(struct ifreq);

                        while (intrface-- > 0)

                        {

                                if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface])))

                                {
                                		if(strcmp(buf[intrface].ifr_name,"eth0")==0){
                                        	ip=(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
                                        	//printf("%s: %s\n", buf[intrface].ifr_name,ip);
                                        	sprintf(ip,"ip_addr %s",ip);
                                		}

                                }

                        }

                }

        close(fd);

        return ip;

        }
		return ip;
}
 
int main(int argc, char **argv)
{
	struct sockaddr_in localaddr,client;
	int sockfd = 0;
	int num = 0;
	char msg[LEN] = {0};
	char *ip = NULL;
	socklen_t sin_size;
 
	if (argc != 2)
	{
		printf("Usage: %s<PORT>\n", argv[0]);
		return -1;
	}
 
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		printf("Create socket fails!\n");
		return -1;
	}
 
	bzero(&localaddr, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(1234);
	//printf("localaddr sinport %d\n",localaddr.sin_port);
	localaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
#if 1
	client.sin_family = AF_INET;
	client.sin_port = htons(1234);
	//printf("localaddr sinport %d\n",localaddr.sin_port);
	client.sin_addr.s_addr = htonl(INADDR_BROADCAST);
 #endif
	int opt = 1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR | SO_BROADCAST,&opt,sizeof(opt));
	//setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
 #if 1
	if (bind(sockfd, (struct sockaddr *)&localaddr, sizeof(struct sockaddr_in)) < 0)
	{
		printf("bind fails!\n");
		return -1;
	}
#endif
	sin_size=sizeof(struct sockaddr_in);
	while(1){
		num = recvfrom(sockfd,msg,LEN,0,(struct sockaddr *)&client,&sin_size);
		if (num <= 0)
		{
			printf("read message fails!\n");
			//return -1;
		}
		//printf("from %s\n",inet_ntoa(client.sin_addr) );
		//printf("from %d\n",client.sin_port);
		//msg[LEN] = '\0';
		//printf("msg %s\n",msg);
		if((num = strcmp(msg,"ip_req")) >= 0){
			ip = get_local_ip();
			//printf("ip %s\n",ip);
			sendto(sockfd, ip, strlen(ip)+8, 0, (struct sockaddr *)&localaddr, sizeof(struct sockaddr_in));
			perror("sendto");
		}
	 	bzero(msg,LEN);
	}
	//printf("time:%s\n", msg);
 
	close(sockfd);
 
	return 1;
}
