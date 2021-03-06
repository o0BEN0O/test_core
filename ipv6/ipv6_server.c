#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 5000
#define MESSAGE "hello"

int main(void)
{
  int sock, conn;
  socklen_t clilen;
  struct sockaddr_in6 server_addr, client_addr;
  char client_ipaddr[INET6_ADDRSTRLEN];

  /* create a STREAM (TCP) socket in the INET6 (IPv6) protocol */
  sock = socket(PF_INET6, SOCK_STREAM, 0);

  if (sock < 0) {
    perror("creating socket");
    exit(1);
  }

#ifdef V6ONLY
  // setting this means the socket only accepts connections from v6;
  // unset, it accepts v6 and v4 (mapped address) connections
	int opt = 1;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) < 0) {
      perror("setting option IPV6_V6ONLY");
      exit(1);
    }
#endif

  /* create server address: this will say where we will be willing to
     accept connections from */

  /* clear it out */
  memset(&server_addr, 0, sizeof(server_addr));

  /* it is an INET6 address */
  server_addr.sin6_family = AF_INET6;

  /* the client IP address, in network byte order */
  /* in this example we accept connections from ANYwhere */
  server_addr.sin6_addr = in6addr_any;

  /* the port we are going to listen on, in network byte order */
  server_addr.sin6_port = htons(PORT);

  /* associate the socket with the address and port */
  if (bind(sock, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit(2);
  }

  /* start the socket listening for new connections */
  if (listen(sock, 5) < 0) {
    perror("listen failed");
    exit(3);
  }

  while (1) {

    /* now wait until we get a connection */
    printf("waiting for a connection...\n");
    clilen = sizeof(client_addr);
    conn = accept(sock, (struct sockaddr *)&client_addr, &clilen);

    if (conn < 0) {
		perror("accept failed");
		exit(4);
    }

    /* now client_addr contains the address of the client */
    printf("connection from %s\n",
           inet_ntop(AF_INET6, &client_addr.sin6_addr, client_ipaddr,
                     INET6_ADDRSTRLEN));
	printf("client addr %s\n",client_ipaddr);

    printf("sending message\n");

    write(conn, MESSAGE, sizeof(MESSAGE));

    /* close connection */
    close(conn);
  }

  return 0;
}
