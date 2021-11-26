#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <linux/in_route.h>
#include <linux/icmpv6.h>
#include <errno.h>
#include <arpa/inet.h>
#include <linux/if_addrlabel.h>
#include <stdbool.h>


struct rtnl_handle {
	int			fd;
	struct sockaddr_nl	local;
	struct sockaddr_nl	peer;
	__u32			seq;
	__u32			dump;
	int			proto;
	FILE		       *dump_fp;
#define RTNL_HANDLE_F_LISTEN_ALL_NSID		0x01
#define RTNL_HANDLE_F_SUPPRESS_NLERR		0x02
#define RTNL_HANDLE_F_STRICT_CHK		0x04
	int			flags;
};

struct rtnl_handle rth = { .fd = -1 };

typedef int (*nl_ext_ack_fn_t)(const char *errmsg, uint32_t off,
			       const struct nlmsghdr *inner_nlh);


int rcvbuf = 1024 * 1024;

#define GW_MAX_LEN 15


#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

enum {
	PREFIXLEN_SPECIFIED	= (1 << 0),
	ADDRTYPE_INET		= (1 << 1),
	ADDRTYPE_UNSPEC		= (1 << 2),
	ADDRTYPE_MULTI		= (1 << 3),

	ADDRTYPE_INET_UNSPEC	= ADDRTYPE_INET | ADDRTYPE_UNSPEC,
	ADDRTYPE_INET_MULTI	= ADDRTYPE_INET | ADDRTYPE_MULTI
};

typedef __u32 __bitwise __be32;

#define MPLS_LS_LABEL_MASK      0xFFFFF000
#define MPLS_LS_LABEL_SHIFT     12
#define MPLS_LS_TC_MASK         0x00000E00
#define MPLS_LS_TC_SHIFT        9
#define MPLS_LS_S_MASK          0x00000100
#define MPLS_LS_S_SHIFT         8
#define MPLS_LS_TTL_MASK        0x000000FF
#define MPLS_LS_TTL_SHIFT       0


struct mpls_label {
	__be32 entry;
};

typedef struct
{
	__u16 flags;
	__u16 bytelen;
	__s16 bitlen;
	/* These next two fields match rtvia */
	__u16 family;
	__u32 data[64];
} inet_prefix;

#define BUFSIZE 8192


struct route_info{
u_int dstAddr;
u_int srcAddr;
u_int gateWay;
char ifName[IF_NAMESIZE];
};


int get_addr_1(inet_prefix *addr, const char *name, int family);

int readNlSock(int sockFd, char *bufPtr, int seqNum, int pId)
{
	struct nlmsghdr *nlHdr;
	int readLen = 0, msgLen = 0;

	do
	{
	//收到内核的应答
		if((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0)
	{
		perror("SOCK READ: ");
		return -1;
	}

	nlHdr = (struct nlmsghdr *)bufPtr;

	//检查header是否有效
	if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))
	{
		perror("Error in recieved packet");
		return -1;
	}

	if(nlHdr->nlmsg_type == NLMSG_DONE)
	{
		break;
	}
	else
	{
		bufPtr += readLen;
		msgLen += readLen;
	}

	if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)
	{
		break;
	}

	}while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));

	return msgLen;
}


//分析返回的路由信息
void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo,char *gateway)
{
	struct rtmsg *rtMsg;
	struct rtattr *rtAttr;
	int rtLen;
	char *tempBuf = NULL;
	struct in_addr dst;
	struct in_addr gate;
	tempBuf = (char *)malloc(100);
	rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);
	//memset(gateway,0,GW_MAX_LEN);

	// If the route is not for AF_INET or does not belong to main routing table
	//then return.
	if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
	{
		return;
	}

	rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
	rtLen = RTM_PAYLOAD(nlHdr);

	for(;RTA_OK(rtAttr,rtLen);rtAttr = RTA_NEXT(rtAttr,rtLen))
	{
		switch(rtAttr->rta_type)
		{
		case RTA_OIF:
			if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
		break;
		case RTA_GATEWAY:
			rtInfo->gateWay = *(u_int *)RTA_DATA(rtAttr);
			//printf("%s[%d] rtInfo->gateWay %d\n",rtInfo->gateWay);
		break;
		case RTA_PREFSRC:
			rtInfo->srcAddr = *(u_int *)RTA_DATA(rtAttr);
		break;
		case RTA_DST:
			rtInfo->dstAddr = *(u_int *)RTA_DATA(rtAttr);
		break;
		}
	}

	dst.s_addr = rtInfo->dstAddr;
	if(strstr((char *)inet_ntoa(dst), "0.0.0.0")&&strncmp(rtInfo->ifName,"eth0",4)==0)
	{
		printf("oif:%s\n",rtInfo->ifName);
		gate.s_addr = rtInfo->gateWay;
		sprintf(gateway, (char *)inet_ntoa(gate));
		//printf("gw%s\n",gateway);
		gate.s_addr = rtInfo->srcAddr;
		//printf("src:%s\n",(char *)inet_ntoa(gate));
		gate.s_addr = rtInfo->dstAddr;
		//printf("dst:%s\n",(char *)inet_ntoa(gate));
	}

	free(tempBuf);
	return;
}


int get_gateway(char *gateway)
{
	struct nlmsghdr *nlMsg;
	struct rtmsg *rtMsg;
	struct route_info *rtInfo;
	char msgBuf[BUFSIZE];
	int sock, len, msgSeq = 0;

	if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
	{
		perror("Socket Creation: ");
		return -1;
	}

	memset(msgBuf, 0, BUFSIZE);
	//memset(gateway,0,GW_MAX_LEN);
	nlMsg = (struct nlmsghdr *)msgBuf;
	rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);
	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.
	nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .
	nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
	nlMsg->nlmsg_seq = msgSeq++; // Sequence of the message packet.
	nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.

	printf("seq %d\n",nlMsg->nlmsg_seq);

	if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)
	{
		printf("Write To Socket Failed…\n");
		return -1;
	}


	if((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0)
	{
			printf("Read From Socket Failed…\n");
			return -1;
			}

	rtInfo = (struct route_info *)malloc(sizeof(struct route_info));

	for(;NLMSG_OK(nlMsg,len);nlMsg = NLMSG_NEXT(nlMsg,len))
	{
			memset(rtInfo, 0, sizeof(struct route_info));
			parseRoutes(nlMsg, rtInfo,gateway);
	}

	free(rtInfo);
	close(sock);
	return 0;
}


static void set_address_type(inet_prefix *addr)
{
	switch (addr->family) {
	case AF_INET:
		if (!addr->data[0])
			addr->flags |= ADDRTYPE_INET_UNSPEC;
		else if (IN_MULTICAST(ntohl(addr->data[0])))
			addr->flags |= ADDRTYPE_INET_MULTI;
		else
			addr->flags |= ADDRTYPE_INET;
		break;
	case AF_INET6:
		if (IN6_IS_ADDR_UNSPECIFIED(addr->data))
			addr->flags |= ADDRTYPE_INET_UNSPEC;
		else if (IN6_IS_ADDR_MULTICAST(addr->data))
			addr->flags |= ADDRTYPE_INET_MULTI;
		else
			addr->flags |= ADDRTYPE_INET;
		break;
	}
}

static int get_addr_ipv4(__u8 *ap, const char *cp)
{
	int i;

	for (i = 0; i < 4; i++) {
		unsigned long n;
		char *endp;

		n = strtoul(cp, &endp, 0);
		if (n > 255)
			return -1;	/* bogus network value */

		if (endp == cp) /* no digits */
			return -1;

		ap[i] = n;

		if (*endp == '\0')
			break;

		if (i == 3 || *endp != '.')
			return -1;	/* extra characters */
		cp = endp + 1;
	}

	return 1;
}

int get_addr_1(inet_prefix *addr, const char *name, int family)
{
	int ret;

	memset(addr, 0, sizeof(*addr));

	addr->family = AF_INET;
	if (family != AF_UNSPEC && family != AF_INET)
		return -1;

	if (get_addr_ipv4((__u8 *)addr->data, name) <= 0)
		return -1;

	addr->bytelen = 4;
	addr->bitlen = -1;


	set_address_type(addr);
	return 0;
}

int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
	      int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta;

	if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
		fprintf(stderr,
			"addattr_l ERROR: message exceeded bound of %d\n",
			maxlen);
		return -1;
	}
	rta = NLMSG_TAIL(n);
	rta->rta_type = type;
	rta->rta_len = len;
	if (alen)
		memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
	//printf("%s[%d]:\n",__func__,__LINE__,data);
	return 0;
}

void rtnl_set_strict_dump(struct rtnl_handle *rth)
{
  int one = 1;

  if (setsockopt(rth->fd, SOL_NETLINK, 12,
			 &one, sizeof(one)) < 0)
	  return;

  rth->flags |= RTNL_HANDLE_F_STRICT_CHK;
}

void rtnl_close(struct rtnl_handle *rth)
{
	  if (rth->fd >= 0) {
		  close(rth->fd);
		  rth->fd = -1;
	  }
}

int rtnl_open_byproto(struct rtnl_handle *rth, unsigned int subscriptions,
				int protocol)
{
	  socklen_t addr_len;
	  int sndbuf = 32768;
	  int one = 1;

	  memset(rth, 0, sizeof(*rth));

	  rth->proto = protocol;
	  rth->fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, protocol);
	  if (rth->fd < 0) {
		  perror("Cannot open netlink socket");
		  return -1;
	  }

	  if (setsockopt(rth->fd, SOL_SOCKET, SO_SNDBUF,
				 &sndbuf, sizeof(sndbuf)) < 0) {
		  perror("SO_SNDBUF");
		  return -1;
	  }

	  if (setsockopt(rth->fd, SOL_SOCKET, SO_RCVBUF,
				 &rcvbuf, sizeof(rcvbuf)) < 0) {
		  perror("SO_RCVBUF");
		  return -1;
	  }

	  /* Older kernels may no support extended ACK reporting */
	  setsockopt(rth->fd, SOL_NETLINK, NETLINK_EXT_ACK,
			 &one, sizeof(one));

	  memset(&rth->local, 0, sizeof(rth->local));
	  rth->local.nl_family = AF_NETLINK;
	  rth->local.nl_groups = subscriptions;

	  if (bind(rth->fd, (struct sockaddr *)&rth->local,
		   sizeof(rth->local)) < 0) {
		  perror("Cannot bind netlink socket");
		  return -1;
	  }
	  addr_len = sizeof(rth->local);
	  if (getsockname(rth->fd, (struct sockaddr *)&rth->local,
			  &addr_len) < 0) {
		  perror("Cannot getsockname");
		  return -1;
	  }
	  if (addr_len != sizeof(rth->local)) {
		  fprintf(stderr, "Wrong address length %d\n", addr_len);
		  return -1;
	  }
	  if (rth->local.nl_family != AF_NETLINK) {
		  fprintf(stderr, "Wrong address family %d\n",
			  rth->local.nl_family);
		  return -1;
	  }
	  rth->seq = time(NULL);
	  return 0;
}

int rtnl_open(struct rtnl_handle *rth, unsigned int subscriptions)
{
	return rtnl_open_byproto(rth, subscriptions, NETLINK_ROUTE);
}

int read_family(const char *name)
{
	int family = AF_UNSPEC;

	if (strcmp(name, "inet") == 0)
		family = AF_INET;
	else if (strcmp(name, "inet6") == 0)
		family = AF_INET6;
	else if (strcmp(name, "link") == 0)
		family = AF_PACKET;
	else if (strcmp(name, "ipx") == 0)
		family = AF_IPX;
	else if (strcmp(name, "mpls") == 0)
		family = AF_MPLS;
	else if (strcmp(name, "bridge") == 0)
		family = AF_BRIDGE;
	return family;
}

static int __rtnl_talk_iov(struct rtnl_handle *rtnl, struct iovec *iov,
			   size_t iovlen, struct nlmsghdr **answer,
			   bool show_rtnl_err, nl_ext_ack_fn_t errfn)
{
	struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
	struct iovec riov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = iov,
		.msg_iovlen = iovlen,
	};
	unsigned int seq = 0;
	struct nlmsghdr *h;
	int i, status;
	char *buf;

	for (i = 0; i < iovlen; i++) {
		h = iov[i].iov_base;
		h->nlmsg_seq = seq = ++rtnl->seq;
		if (answer == NULL)
			h->nlmsg_flags |= NLM_F_ACK;
	}

	printf("%s[%d]: iovlen %ld nladdr %d\n",__func__,__LINE__,iovlen,nladdr);

	printf("%s[%d] flag %ld type %ld\n",__func__,__LINE__,h->nlmsg_flags,h->nlmsg_type);

	status = sendmsg(rtnl->fd, &msg, 0);
	if (status < 0) {
		perror("Cannot talk to rtnetlink");
		return -1;
	}

	return 0;
}


static int __rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n,
		       struct nlmsghdr **answer,
		       bool show_rtnl_err, nl_ext_ack_fn_t errfn)
{
	struct iovec iov = {
		.iov_base = n,
		.iov_len = n->nlmsg_len
	};

	return __rtnl_talk_iov(rtnl, &iov, 1, answer, show_rtnl_err, errfn);
}


int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n,
	      struct nlmsghdr **answer)
{
	return __rtnl_talk(rtnl, n, answer, true, NULL);
}

static int iproute_modify(int cmd, unsigned int flags,char *gateway)
{
	struct {
	struct nlmsghdr	n;
	struct rtmsg		r;
		char			buf[4096];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | flags,
		.n.nlmsg_type = cmd,
		.r.rtm_family = AF_UNSPEC,
		.r.rtm_table = RT_TABLE_MAIN,
		.r.rtm_scope = RT_SCOPE_NOWHERE,
	};

	inet_prefix addr;
	int family;

	family = read_family(gateway);
	printf("family %d, unspec %d\n",family,AF_UNSPEC);
	if (family == AF_UNSPEC)
		family = req.r.rtm_family;

	get_addr_1(&addr, gateway, family);//利用ip地址和family来获得addr
	if (req.r.rtm_family == AF_UNSPEC)
		req.r.rtm_family = addr.family;
	printf("%s[%d]:addr.family %d req.r.rtm_family %d\n",__func__,__LINE__,addr.family,req.r.rtm_family);

	if (addr.family == req.r.rtm_family)
		addattr_l(&req.n, sizeof(req), RTA_GATEWAY,
			  &addr.data, addr.bytelen);

	req.r.rtm_type=0;

	printf("%s[%d] family %d table %d  type %d scope %d \n",__func__,__LINE__,
		req.r.rtm_family,req.r.rtm_table,req.r.rtm_type,req.r.rtm_scope);

	if (rtnl_talk(&rth, &req.n, NULL) < 0)
		return -2;
	return 0;
}



int _del_gw(char *gateway)
{
	int ret=-1;
	if (rtnl_open(&rth, 0) < 0){
		perror("open fail");
		return ret;
	}
	rtnl_set_strict_dump(&rth);
	ret=iproute_modify(RTM_DELROUTE,0,gateway);
	return ret;
}


int main(void)
{
	int ret=-1;
	char buff[GW_MAX_LEN];
	memset(buff,0,sizeof(buff));
	get_gateway(buff);
	while(strlen(buff)!=0){
		printf("gw = %s\n",buff);
		ret=_del_gw(buff);
		memset(buff,0,sizeof(buff));
		get_gateway(buff);
		rtnl_close(&rth);
	}
	return ret;
}
