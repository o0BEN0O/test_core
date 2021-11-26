/*
 * dhcpcd - DHCP client daemon
 * Copyright (c) 2017 Raymarine
 * All rights reserved

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef IPV4LL_GROUP_H
#define IPV4LL_GROUP_H

#ifdef IPV4LL
#include "arp.h"

#include "config.h"

#ifdef HAVE_SYS_QUEUE_H
#include <sys/queue.h>
#endif

struct ipv4llg_state {
	struct ipv4_addr *addr;
	struct arp_state *arp;
	unsigned int conflicts;
	struct timespec defend;
	char randomstate[128];
	uint8_t down;
	struct ipv4ll_group *group;
};

#define IN_LINKLOCAL_GROUP(addr, group)						\
	(addr >= ntohl(group->range_begin.s_addr) &&				\
	 addr <= ntohl(group->range_end.s_addr))

#define	IPV4LLG_STATE(ifp, idx)						       \
	(&((struct ipv4llg_state *)(ifp)->if_data[IF_DATA_IPV4LLG])[idx])
#define	IPV4LLG_CSTATE(ifp, idx)					       \
	(&((const struct ipv4llg_state *)(ifp)->if_data[IF_DATA_IPV4LLG])[idx])
#define	IPV4LLG_STATE_RUNNING(ifp, idx)					       \
	(IPV4LLG_CSTATE((ifp), (idx)) && !IPV4LLG_CSTATE((ifp), (idx))->down && \
	(IPV4LLG_CSTATE((ifp), (idx))->addr != NULL))

int ipv4llg_subnetroute(struct rt_head *, struct interface *);
int ipv4llg_defaultroute(struct rt_head *,struct interface *);
ssize_t ipv4llg_env(char **, const char *, const struct interface *);
void ipv4llg_start(void *);
void ipv4llg_claimed(void *);
void ipv4llg_handle_failure(void *);
#ifdef HAVE_ROUTE_METRIC
int ipv4llg_recvrt(int, const struct rt *);
#endif

#define	ipv4llg_free(ifp)		ipv4llg_freedrop((ifp), 0);
#define	ipv4llg_drop(ifp)		ipv4llg_freedrop((ifp), 1);
void ipv4llg_freedrop(struct interface *, int);
void ipv4llg_writeclaims(void *);
#else
#define	IPV4LLG_STATE_RUNNING(ifp)	(0)
#define	ipv4llg_subnetroute(route, ifp)	(0)
#define	ipv4llg_defaultroute(route, ifp)	(0)
#define	ipv4llg_handlert(a, b, c)	(0)
#define	ipv4llg_free(a)			{}
#define	ipv4llg_drop(a)			{}
#define ipv4llg_writeclaims(a)		{}
#endif

#endif
