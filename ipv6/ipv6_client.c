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

#define MAXBUF 1024
int main(int argc, char *argv[])
{
    int iFd;
    char arrcBuffer[MAXBUF + 1];
    char *pcIPV6 = "fe80::202:ff:fe04:b3";
    char *pcDev = "br-wan";
    int iPort = 5000;

    //printf("PAR: IPV6=%s DEV=%s PORT=%d \n",pcIPV6, pcDev, iPort);
    //if (argc < 3)
    //{
    //	printf("usage: %s  netdev  ipv6_addr \n", argv[0]);
    //	return 1;
    //}

    iFd = socket(AF_INET6, SOCK_STREAM, 0);
    if (iFd < 0)
    {
        perror("socket");
        return 1;
    }

    struct ifreq tReq;
    strcpy(tReq.ifr_name, pcDev);
    if (ioctl(iFd, SIOCGIFINDEX, &tReq) < 0)
    {
        perror("SIOCGIFINDEX");
        return 2;
    }

    struct sockaddr_in6 tDestIPV6 = {AF_INET6, htons(iPort), 0};
    inet_pton(AF_INET6, pcIPV6, &tDestIPV6.sin6_addr);
    tDestIPV6.sin6_scope_id = tReq.ifr_ifindex;

    if (connect(iFd, (struct sockaddr*)&tDestIPV6, sizeof(tDestIPV6)) < 0)
    {
        perror("connect");
        return 3;
    }
    printf("connect OK IPV6=%s DEV=%s PORT=%d\n",pcIPV6, pcDev, iPort);

    /* 接收服务器来的消息 */
    int iLen = recv(iFd, arrcBuffer, MAXBUF, 0);
    if (iLen > 0)
        printf("Rcv Msg=%s \r\n Len = %d\r\n",arrcBuffer, iLen);
    else
        printf("Rcv ERR. CODE=%d MSG = %s\n",errno, strerror(errno));

    bzero(arrcBuffer, MAXBUF + 1);
    strcpy(arrcBuffer, "client ipv6 123adf\r\n");
    /* 发消息给服务器 */
    iLen = send(iFd, arrcBuffer, strlen(arrcBuffer), 0);
    if (iLen < 0)
        printf("Snd ERR. CODE=%d  MSG = %s\n",errno, strerror(errno));
    else
        printf("Send Msg=%s Len = %d\r\n",arrcBuffer, iLen);

    close(iFd);
    return 0;

}
