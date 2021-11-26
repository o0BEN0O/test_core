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
#include <fnmatch.h>

typedef struct
{
	__u16 flags;
	__u16 bytelen;
	__s16 bitlen;
	/* These next two fields match rtvia */
	__u16 family;
	__u32 data[64];
} inet_prefix;

struct link_filter {
	int ifindex;
	int family;
	int oneline;
	int showqueue;
	inet_prefix pfx;
	int scope, scopemask;
	int flags, flagmask;
	int up;
	char *label;
	int flushed;
	char *flushb;
	int flushp;
	int flushe;
	int group;
	int master;
	char *kind;
	char *slave_kind;
};

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

#define BUFSIZE 8192

#define SPRINT_BSIZE 64
#define SPRINT_BUF(x)	char x[SPRINT_BSIZE]

struct route_info{
u_int dstAddr;
u_int srcAddr;
u_int gateWay;
char ifName[IF_NAMESIZE];
};

static struct link_filter filter;
int max_flush_loops = 10;
int show_stats=0;

int flush_update(struct rtnl_handle* rth);


typedef int (*rtnl_filter_t)(struct rtnl_handle *rth,const struct sockaddr_nl *,
			     struct nlmsghdr *n, void *);

struct rtnl_dump_filter_arg {
	rtnl_filter_t filter;
	void *arg1;
	__u16 nc_flags;
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

static int __ipaddr_rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n,
		       struct nlmsghdr *answer, size_t maxlen,
		       bool show_rtnl_err, nl_ext_ack_fn_t errfn)
{
	int status;
	unsigned int seq;
	struct nlmsghdr *h;
	struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
	struct iovec iov = {
		.iov_base = n,
		.iov_len = n->nlmsg_len
	};
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	n->nlmsg_seq = seq = ++rtnl->seq;

	if (answer == NULL)
		n->nlmsg_flags |= NLM_F_ACK;

	status = sendmsg(rtnl->fd, &msg, 0);
	if (status < 0) {
		perror("Cannot talk to rtnetlink");
		return -1;
	}
	return 0;
}


int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n,
	      struct nlmsghdr **answer)
{
	return __rtnl_talk(rtnl, n, answer, true, NULL);
}

int ipaddr_rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n,
	      struct nlmsghdr *answer)
{
	return __ipaddr_rtnl_talk(rtnl, n, answer, 0,true, NULL);
}

static int iproute_modify(struct rtnl_handle *rth,int cmd, unsigned int flags,char *gateway)
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


	if (rtnl_talk(rth, &req.n, NULL) < 0)
		return -2;
	return 0;
}



int _del_gw(char *gateway)
{
	int ret=-1;
	struct rtnl_handle rth = { .fd = -1 };
	if (rtnl_open(&rth, 0) < 0){
		perror("open fail");
		return ret;
	}
	rtnl_set_strict_dump(&rth);
	ret=iproute_modify(&rth,RTM_DELROUTE,0,gateway);
	rtnl_close(&rth);
	return ret;
}

int ipv6_ipaddr_modify(struct rtnl_handle *ipv6_addr_rth,const char *dev, const char* ipv6_addr,int netmask)
{
	int ret=0;
	struct {
		struct nlmsghdr	n;
		struct ifaddrmsg	ifa;
		char			buf[256];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE|NLM_F_EXCL,
		.n.nlmsg_type = RTM_NEWADDR,
		.ifa.ifa_family = AF_INET6,
	};
	inet_prefix lcl = {};
	int i=0;

	req.ifa.ifa_family=AF_INET6;/*AF_INET6 10*/
	printf("ipv6_addr [%s]\n",ipv6_addr);
	ret=inet_pton(AF_INET6, ipv6_addr, lcl.data);
	if(ret==0||ret<0){
		printf("ipv6_addr[%s] is invalid\n",ipv6_addr);
		return -1;
	}
	lcl.bytelen = 16;

	addattr_l(&req.n, sizeof(req), IFA_LOCAL, &lcl.data, lcl.bytelen);
	req.ifa.ifa_flags=0;

	addattr_l(&req.n, sizeof(req), IFA_ADDRESS, &lcl.data, lcl.bytelen);

	req.ifa.ifa_prefixlen=netmask;/*netmask length?*/
	req.ifa.ifa_scope=0;

	req.ifa.ifa_index=if_nametoindex(dev);


	ipaddr_rtnl_talk(ipv6_addr_rth, &req.n, NULL);
	return 0;
}


int IPv6_address_add(const char* dev,const char* ipv6_addr,int netmask)
{
	int ret=-1;
	struct rtnl_handle ipv6_addr_rth = { .fd = -1 };

	ret=rtnl_open(&ipv6_addr_rth, 0);
	if(ret<0){
		printf("rtnl_open failed\n");
		return -1;
	}

	ret=ipv6_ipaddr_modify(&ipv6_addr_rth,dev,ipv6_addr,netmask);
	rtnl_close(&ipv6_addr_rth);
	return ret;
}

int inet_addr_match(const inet_prefix *a, const inet_prefix *b, int bits)
{
	const __u32 *a1 = a->data;
	const __u32 *a2 = b->data;
	int words = bits >> 0x05;

	bits &= 0x1f;

	if (words)
		if (memcmp(a1, a2, words << 2))
			return -1;

	if (bits) {
		__u32 w1, w2;
		__u32 mask;

		w1 = a1[words];
		w2 = a2[words];

		mask = htonl((0xffffffff) << (0x20 - bits));

		if ((w1 ^ w2) & mask)
			return 1;
	}

	return 0;
}

const char *ll_index_to_name(unsigned idx)
{
	static char nbuf[IFNAMSIZ];

	if (if_indextoname(idx, nbuf) == NULL)
		snprintf(nbuf, IFNAMSIZ, "if%d", idx);

	return nbuf;
}


const char *ll_idx_n2a(unsigned idx, char *buf)
{
	if (if_indextoname(idx, buf) == NULL)
		snprintf(buf, IFNAMSIZ, "if%d", idx);
	return buf;
}

int parse_rtattr_flags(struct rtattr *tb[], int max, struct rtattr *rta,
		       int len, unsigned short flags)
{
	unsigned short type;

	memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
	while (RTA_OK(rta, len)) {
		type = rta->rta_type & ~flags;
		if ((type <= max) && (!tb[type]))
			tb[type] = rta;
		rta = RTA_NEXT(rta, len);
	}
	if (len)
		fprintf(stderr, "!!!Deficit %d, rta_len=%d\n",
			len, rta->rta_len);
	return 0;
}

int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	return parse_rtattr_flags(tb, max, rta, len, 0);
}

static inline __u32 rta_getattr_u32(const struct rtattr *rta)
{
	return *(__u32 *)RTA_DATA(rta);
}

static unsigned int get_ifa_flags(struct ifaddrmsg *ifa,
				 struct rtattr *ifa_flags_attr)
{
   return ifa_flags_attr ? rta_getattr_u32(ifa_flags_attr) :
	   ifa->ifa_flags;
}


int print_addrinfo(struct rtnl_handle *rth,const struct sockaddr_nl *who, struct nlmsghdr *n,
		   void *arg)
{
	FILE *fp = arg;
	struct ifaddrmsg *ifa = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	unsigned int ifa_flags;
	struct rtattr *rta_tb[IFA_MAX+1];

	SPRINT_BUF(b1);

	if (n->nlmsg_type != RTM_NEWADDR && n->nlmsg_type != RTM_DELADDR)
		return 0;
	len -= NLMSG_LENGTH(sizeof(*ifa));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	if (filter.flushb && n->nlmsg_type != RTM_NEWADDR)
		return 0;

	parse_rtattr(rta_tb, IFA_MAX, IFA_RTA(ifa),
		     n->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa)));

	ifa_flags = get_ifa_flags(ifa, rta_tb[IFA_FLAGS]);

	if (!rta_tb[IFA_LOCAL])
		rta_tb[IFA_LOCAL] = rta_tb[IFA_ADDRESS];
	if (!rta_tb[IFA_ADDRESS])
		rta_tb[IFA_ADDRESS] = rta_tb[IFA_LOCAL];

	if (filter.ifindex && filter.ifindex != ifa->ifa_index)
		return 0;
	if ((filter.scope^ifa->ifa_scope)&filter.scopemask)
		return 0;
	if ((filter.flags ^ ifa_flags) & filter.flagmask)
		return 0;
	if (filter.label) {
		SPRINT_BUF(b1);
		const char *label;

		if (rta_tb[IFA_LABEL])
			label = RTA_DATA(rta_tb[IFA_LABEL]);
		else
			label = ll_idx_n2a(ifa->ifa_index, b1);
		if (fnmatch(filter.label, label, 0) != 0)
			return 0;
	}
	if (filter.pfx.family) {
		if (rta_tb[IFA_LOCAL]) {
			inet_prefix dst = { .family = ifa->ifa_family };

			memcpy(&dst.data, RTA_DATA(rta_tb[IFA_LOCAL]), RTA_PAYLOAD(rta_tb[IFA_LOCAL]));
			if (inet_addr_match(&dst, &filter.pfx, filter.pfx.bitlen))
				return 0;
		}
	}

	if (filter.family && filter.family != ifa->ifa_family)
		return 0;

	if (filter.flushb) {
		struct nlmsghdr *fn;

		if (NLMSG_ALIGN(filter.flushp) + n->nlmsg_len > filter.flushe) {
			if (flush_update(rth))
				return -1;
		}
		fn = (struct nlmsghdr *)(filter.flushb + NLMSG_ALIGN(filter.flushp));
		memcpy(fn, n, n->nlmsg_len);
		fn->nlmsg_type = RTM_DELADDR;
		fn->nlmsg_flags = NLM_F_REQUEST;
		fn->nlmsg_seq = ++rth->seq;
		filter.flushp = (((char *)fn) + n->nlmsg_len) - filter.flushb;
		filter.flushed++;
		if (show_stats < 2)
			return 0;
	}

	return 0;
}


static int rtnl_dump_done(struct nlmsghdr *h)
{
	int len = *(int *)NLMSG_DATA(h);

	if (h->nlmsg_len < NLMSG_LENGTH(sizeof(int))) {
		fprintf(stderr, "DONE truncated\n");
		return -1;
	}

	if (len < 0) {
		errno = -len;
		switch (errno) {
		case ENOENT:
		case EOPNOTSUPP:
			return -1;
		case EMSGSIZE:
			fprintf(stderr,
				"Error: Buffer too small for object.\n");
			break;
		default:
			perror("RTNETLINK answers");
		}
		return len;
	}

	return 0;
}

static void rtnl_dump_error(const struct rtnl_handle *rth,
			    struct nlmsghdr *h)
{

	if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
		fprintf(stderr, "ERROR truncated\n");
	} else {
		const struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(h);

		errno = -err->error;
		if (rth->proto == NETLINK_SOCK_DIAG &&
		    (errno == ENOENT ||
		     errno == EOPNOTSUPP))
			return;

		if (!(rth->flags & RTNL_HANDLE_F_SUPPRESS_NLERR))
			perror("RTNETLINK answers");
	}
}

int rtnl_dump_filter_l(struct rtnl_handle *rth,
		       const struct rtnl_dump_filter_arg *arg)
{
	struct sockaddr_nl nladdr;
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	char buf[32768];
	int dump_intr = 0;

	iov.iov_base = buf;
	while (1) {
		int status;
		const struct rtnl_dump_filter_arg *a;
		int found_done = 0;
		int msglen = 0;

		iov.iov_len = sizeof(buf);
		status = recvmsg(rth->fd, &msg, 0);

		if (status < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			fprintf(stderr, "netlink receive error %s (%d)\n",
				strerror(errno), errno);
			return -1;
		}

		if (status == 0) {
			fprintf(stderr, "EOF on netlink\n");
			return -1;
		}

		if (rth->dump_fp)
			fwrite(buf, 1, NLMSG_ALIGN(status), rth->dump_fp);

		for (a = arg; a->filter; a++) {
			struct nlmsghdr *h = (struct nlmsghdr *)buf;

			msglen = status;

			while (NLMSG_OK(h, msglen)) {
				int err = 0;

				h->nlmsg_flags &= ~a->nc_flags;

				if (nladdr.nl_pid != 0 ||
				    h->nlmsg_pid != rth->local.nl_pid ||
				    h->nlmsg_seq != rth->dump)
					goto skip_it;

				if (h->nlmsg_flags & NLM_F_DUMP_INTR)
					dump_intr = 1;

				if (h->nlmsg_type == NLMSG_DONE) {
					err = rtnl_dump_done(h);
					if (err < 0)
						return -1;

					found_done = 1;
					break; /* process next filter */
				}

				if (h->nlmsg_type == NLMSG_ERROR) {
					rtnl_dump_error(rth, h);
					return -1;
				}

				if (!rth->dump_fp) {
					err = a->filter(rth,&nladdr, h, a->arg1);
					if (err < 0)
						return err;
				}

skip_it:
				h = NLMSG_NEXT(h, msglen);
			}
		}

		if (found_done) {
			if (dump_intr)
				fprintf(stderr,
					"Dump was interrupted and may be inconsistent.\n");
			return 0;
		}

		if (msg.msg_flags & MSG_TRUNC) {
			fprintf(stderr, "Message truncated\n");
			continue;
		}
		if (msglen) {
			fprintf(stderr, "!!!Remnant of size %d\n", msglen);
			exit(1);
		}
	}
}


int rtnl_dump_filter_nc(struct rtnl_handle *rth,
		     rtnl_filter_t filter,
		     void *arg1, __u16 nc_flags)
{
	const struct rtnl_dump_filter_arg a[2] = {
		{ .filter = filter, .arg1 = arg1, .nc_flags = nc_flags, },
		{ .filter = NULL,   .arg1 = NULL, .nc_flags = 0, },
	};

	return rtnl_dump_filter_l(rth, a);
}

int rtnl_wilddump_req_filter(struct rtnl_handle *rth, int family, int type,
			    __u32 filt_mask)
{
	struct {
		struct nlmsghdr nlh;
		struct ifinfomsg ifm;
		/* attribute has to be NLMSG aligned */
		struct rtattr ext_req __attribute__ ((aligned(NLMSG_ALIGNTO)));
		__u32 ext_filter_mask;
	} req = {
		.nlh.nlmsg_len = sizeof(req),
		.nlh.nlmsg_type = type,
		.nlh.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST,
		.nlh.nlmsg_seq = rth->dump = ++rth->seq,
		.ifm.ifi_family = family,
		.ext_req.rta_type = IFLA_EXT_MASK,
		.ext_req.rta_len = RTA_LENGTH(sizeof(__u32)),
		.ext_filter_mask = filt_mask,
	};

	return send(rth->fd, &req, sizeof(req), 0);
}


int rtnl_wilddump_request(struct rtnl_handle *rth, int family, int type)
{
	return rtnl_wilddump_req_filter(rth, family, type, RTEXT_FILTER_VF);
}

int rtnl_send_check(struct rtnl_handle *rth, const void *buf, int len)
{
	struct nlmsghdr *h;
	int status;
	char resp[1024];

	status = send(rth->fd, buf, len, 0);
	if (status < 0)
		return status;

	/* Check for immediate errors */
	status = recv(rth->fd, resp, sizeof(resp), MSG_DONTWAIT|MSG_PEEK);
	if (status < 0) {
		if (errno == EAGAIN)
			return 0;
		return -1;
	}

	for (h = (struct nlmsghdr *)resp; NLMSG_OK(h, status);
	     h = NLMSG_NEXT(h, status)) {
		if (h->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(h);

			if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr)))
				fprintf(stderr, "ERROR truncated\n");
			else
				errno = -err->error;
			return -1;
		}
	}

	return 0;
}


int flush_update(struct rtnl_handle* rth)
{

	/*
	 * Note that the kernel may delete multiple addresses for one
	 * delete request (e.g. if ipv4 address promotion is disabled).
	 * Since a flush operation is really a series of delete requests
	 * its possible that we may request an address delete that has
	 * already been done by the kernel. Therefore, ignore EADDRNOTAVAIL
	 * errors returned from a flush request
	 */
	if ((rtnl_send_check(rth, filter.flushb, filter.flushp) < 0) &&
	    (errno != EADDRNOTAVAIL)) {
		perror("Failed to send flush request");
		return -1;
	}
	filter.flushp = 0;
	return 0;
}


static int ipaddr_flush(struct rtnl_handle* rth)
{
	int round = 0;
	char flushb[4096-512];

	filter.flushb = flushb;
	filter.flushp = 0;
	filter.flushe = sizeof(flushb);

	while ((max_flush_loops == 0) || (round < max_flush_loops)) {

		if (rtnl_wilddump_request(rth, filter.family, RTM_GETADDR) < 0) {
			perror("Cannot send dump request");
			exit(1);
		}
		filter.flushed = 0;
		if (rtnl_dump_filter_nc(rth, print_addrinfo,
					stdout, NLM_F_DUMP_INTR) < 0) {
			fprintf(stderr, "Flush terminated\n");
			exit(1);
		}
		if (filter.flushed == 0) {
 flush_done:
			if (show_stats) {
				if (round == 0)
					printf("Nothing to flush.\n");
				else
					printf("*** Flush is complete after %d round%s ***\n", round, round > 1?"s":"");
			}
			fflush(stdout);
			return 0;
		}
		round++;
		if (flush_update(rth) < 0)
			return 1;

		if (show_stats) {
			printf("\n*** Round %d, deleting %d addresses ***\n", round, filter.flushed);
			fflush(stdout);
		}

		/* If we are flushing, and specifying primary, then we
		 * want to flush only a single round.  Otherwise, we'll
		 * start flushing secondaries that were promoted to
		 * primaries.
		 */
		if (!(filter.flags & IFA_F_SECONDARY) && (filter.flagmask & IFA_F_SECONDARY))
			goto flush_done;
	}
	fprintf(stderr, "*** Flush remains incomplete after %d rounds. ***\n", max_flush_loops);
	fflush(stderr);
	return 1;
}


int IPv6_flush_scope_global_address(const char* dev)
{
	int ret=-1;
	struct rtnl_handle ipv6_addr_rth = { .fd = -1 };
	ret=rtnl_open(&ipv6_addr_rth, 0);
	if(ret<0){
		printf("rtnl_open failed\n");
		return -1;
	}

	memset(&filter, 0, sizeof(filter));
	filter.oneline=0;
	filter.showqueue = 1;
	filter.family = AF_INET6;
	filter.group = -1;
	filter.ifindex=if_nametoindex(dev);
	filter.scopemask=-1;
	filter.scope=0;
	ipaddr_flush(&ipv6_addr_rth);

	rtnl_close(&ipv6_addr_rth);
}

int main(void)
{
	//IPv6_address_add("eth0","fd41:7777:b8ea:1:211:c7ff:fe11:1111",64);
	IPv6_flush_scope_global_address("eth0");
	return 0;
}
