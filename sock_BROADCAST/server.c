#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#define LEN				256


void getTime(char *curTime)
{
	time_t tm;
	time(&tm);
 
	snprintf(curTime, LEN, "%s\n", "ip_req");
}
 
int main(void)
{
	struct sockaddr_in peeraddr,client;
	int sockfd = 0;
	int on = 1;
	int num = 0;
	socklen_t sin_size = 0;
 
	char msg[LEN] = {0};
	char ip[LEN] = {0};

 
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		printf("Create socket fails!\n");
		return -1;
	}
 
	//设置套接字为广播模式  SO_BROADCAST
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
 
	bzero(&peeraddr, sizeof(peeraddr));
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_addr.s_addr=htonl(INADDR_BROADCAST); 
	peeraddr.sin_port = htons(8888);
	#if 0
	if (inet_pton(AF_INET, argv[1], &peeraddr.sin_addr) <= 0)
	{
		printf("IP address is error!\n");
		return -1;
	}
	peeraddr.sin_port = htons(atoi(argv[2]));
	#endif
	
 	sin_size = sizeof(struct sockaddr_in);
	for (;;)
	{
		getTime(msg);
		num = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&peeraddr,sin_size);
		if (num<0)
			perror("sendto");
		printf("num %d\n",num);
 		//bzero(msg,LEN);
		//fflush(stdout);
		sleep(3);
		if ((num=recvfrom(sockfd,ip,LEN,0,(struct sockaddr *)&client,&sin_size)) == -1)
			printf("num %d\n",num);
		printf("ip %s\n",ip);
		//	break;
		//}
		//bzero(ip,LEN);
	}
 
	close(sockfd);
	
	return 1;
}