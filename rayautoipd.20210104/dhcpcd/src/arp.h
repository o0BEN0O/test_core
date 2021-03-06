/*
 * dhcpcd - DHCP client daemon
 * Copyright (c) 2006-2017 Roy Marples <roy@marples.name>
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

#ifndef ARP_H
#define ARP_H

/* ARP timings from RFC5227 */
#define PROBE_WAIT		 1
#define PROBE_NUM		 3
#define PROBE_MIN		 1
#define PROBE_MAX		 2
#define ANNOUNCE_WAIT		 2
#define ANNOUNCE_NUM		 2
#define ANNOUNCE_INTERVAL	 2
#define MAX_CONFLICTS		10
#define RATE_LIMIT_INTERVAL	60
#define DEFEND_INTERVAL		10

/* -- Raymarine -- */
/* See RFC5227 (section 2.6) */
/* Normally, an address clash will not occur, as all Raymarine Auto-IP
   users will probe for other users before allocating ourselves an
   address for active use. However, this may fail to detect a clash if
   there is a temporary failure in a communication link, e.g. due to a
   disconnected cable. Given that the number of individual devices on
   SThs is not large, it's fine to periodically poke the network with
   a broadcast in order to see if anybody else is around who we can
   spot using our own address. */
/* Unlike RFC5227 section 2.6 paragraph 3, we don't use link-level
   broadcast ARP replies, but we instead have a periodic timer here to
   send out Gratuitous ARP Requests for our own Auto-IP addresses. If
   there is an address clash, then either we will lose the defense, or
   the other side will lose. The timer here is set to a conservative
   value and a random spread is added. In most cases, other LNet and
   other multicast/broadcast traffic on the SThs network will be far
   chattier than this value. */
/* Timer selection: the default NMEA 2000 heartbeat PGN 126993
   transmission rate is once a minute, so let's go for something
   similar here, but add a spread factor. */
/* This means: 60s +/- 5s */
#define ANNOUNCE_ALWAYS_WAIT        55
#define ANNOUNCE_ALWAYS_WAIT_SPREAD 11

#include "dhcpcd.h"
#include "if.h"

#ifdef IN_IFF_DUPLICATED
/* NetBSD gained RFC 5227 support in the kernel.
 * This means dhcpcd doesn't need ARP except for ARPing support. */
#if defined(__NetBSD_Version__) && __NetBSD_Version__ >= 799003900
#define KERNEL_RFC5227
#endif
#endif

struct arp_msg {
	uint16_t op;
	unsigned char sha[HWADDR_LEN];
	struct in_addr sip;
	unsigned char tha[HWADDR_LEN];
	struct in_addr tip;
};

struct arp_state {
	TAILQ_ENTRY(arp_state) next;
	struct interface *iface;

	/* -- Raymarine -- */
	void *cookie;
	int always_announce;

	void (*probed_cb)(struct arp_state *);
	void (*announced_cb)(struct arp_state *);
	void (*conflicted_cb)(struct arp_state *, const struct arp_msg *);
	void (*free_cb)(struct arp_state *);

	struct in_addr addr;
	int probes;
	int claims;
	struct in_addr failed;
};
TAILQ_HEAD(arp_statehead, arp_state);

struct iarp_state {
	int fd;
	struct arp_statehead arp_states;
};

#define ARP_STATE(ifp)							       \
	((struct iarp_state *)(ifp)->if_data[IF_DATA_ARP])
#define ARP_CSTATE(ifp)							       \
	((const struct iarp_state *)(ifp)->if_data[IF_DATA_ARP])

#ifdef ARP
int arp_open(struct interface *);
ssize_t arp_request(const struct interface *, in_addr_t, in_addr_t);
void arp_probe(struct arp_state *);
void arp_close(struct interface *);
void arp_report_conflicted(const struct arp_state *, const struct arp_msg *);
struct arp_state *arp_new(struct interface *, const struct in_addr *);
struct arp_state *arp_find(struct interface *, const struct in_addr *);
void arp_announce(struct arp_state *);
void arp_announceaddr(struct dhcpcd_ctx *, struct in_addr *);
void arp_cancel(struct arp_state *);
void arp_free(struct arp_state *);
void arp_free_but(struct arp_state *);
void arp_drop(struct interface *);

void arp_handleifa(int, struct ipv4_addr *);
#endif /* ARP */
#endif /* ARP_H */
