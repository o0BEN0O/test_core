/*
 * dhcpcd - DHCP client daemon
 * Copyright (c) 2017 Raymarine
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

/* Implementation notes:
 *
 * - IN_IFF_TENTATIVE is not in Linux. Code path untested.
 * - I've gone with the trade-off of duplicating code from ipv4ll.c; another
 *   way to do this would be to factor out the common bits some more.
 * - ipv4llg_defaultroute is not implemented as we don't want to set a default
 *   route for any IPv4LLg addresses.
 */

#include <arpa/inet.h>

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ELOOP_QUEUE 7
#include "config.h"
#include "arp.h"
#include "common.h"
#include "eloop.h"
#include "if.h"
#include "if-options.h"
#include "ipv4.h"
#include "ipv4ll.h"
#include "ipv4ll-group.h"
#include "logerr.h"
#include "sa.h"
#include "script.h"

#ifdef IPV4LL

static in_addr_t
ipv4llg_pickaddr(struct arp_state *astate)
{
	struct in_addr addr;
	struct ipv4llg_state *istate;

	istate = astate->cookie;
	setstate(istate->randomstate);

	do {
		long r;

		/* Work out network size */
		uint32_t top = ntohl(istate->group->range_end.s_addr);
		uint32_t bottom = ntohl(istate->group->range_begin.s_addr);
		uint32_t size = top - bottom;

		/* coverity[dont_call] */
		r = random();
		addr.s_addr = htonl(bottom | ((uint32_t)(r % size)));

		/* If we have a preferred address to try first, let's go with that */
		if (istate->group->last_address.s_addr)
		{
			addr.s_addr = istate->group->last_address.s_addr;
			istate->group->last_address.s_addr = 0;
		}

		/* No point using a failed address */
		if (addr.s_addr == astate->failed.s_addr)
			continue;
		/* Ensure we don't have the address on another interface */
	} while (ipv4_findaddr(astate->iface->ctx, &addr) != NULL);

	/* Restore the original random state */
	setstate(istate->arp->iface->ctx->randomstate);
	return addr.s_addr;
}

static int
ipv4llg_subnetroute_one(struct rt_head *routes, struct interface *ifp, struct ipv4llg_state *state)
{
	struct rt *rt;
	struct in_addr in;

	assert(ifp != NULL);
	if (state->addr == NULL)
		return 0;

	if (state->group->no_routes)
		return 0;

	if ((rt = rt_new(ifp)) == NULL)
		return -1;

	in.s_addr = state->addr->addr.s_addr & state->addr->mask.s_addr;
	sa_in_init(&rt->rt_dest, &in);
	in.s_addr = state->addr->mask.s_addr;
	sa_in_init(&rt->rt_netmask, &in);
	in.s_addr = INADDR_ANY;
	sa_in_init(&rt->rt_gateway, &in);
	sa_in_init(&rt->rt_ifa, &state->addr->addr);
	TAILQ_INSERT_TAIL(routes, rt, rt_next);
	return 1;
}

int
ipv4llg_subnetroute(struct rt_head *routes, struct interface *ifp)
{
	int ret = 0;
	size_t idx = 0;
	struct ipv4ll_group *group;

	assert(ifp != NULL);
	if (!ifp->if_data[IF_DATA_IPV4LLG])
		return 0;

	TAILQ_FOREACH(group, &ifp->options->ipv4ll_groups, next) {
		ret |= ipv4llg_subnetroute_one(routes, ifp, IPV4LLG_STATE(ifp, idx++));
	}

	return ret;
}

static void
ipv4llg_probed(struct arp_state *astate)
{
	struct interface *ifp;
	struct ipv4llg_state *state;
	struct ipv4_addr *ia;

	struct in_addr inaddr_mask;
	struct in_addr inaddr_bcast;

	assert(astate != NULL);
	assert(astate->iface != NULL);

	ifp = astate->iface;
	state = astate->cookie;
	assert(state != NULL);

	/* Work out the network mask */
	inaddr_mask = state->group->network;

	/* Work out the broadcast address */
	inaddr_bcast.s_addr = (astate->addr.s_addr & inaddr_mask.s_addr) | ~inaddr_mask.s_addr;

	ia = ipv4_iffindaddr(ifp, &astate->addr, &inaddr_mask);
#ifdef IN_IFF_NOTREADY
	if (ia == NULL || ia->addr_flags & IN_IFF_NOTREADY)
#endif
		loginfox("%s: using IPv4LLg address: group %s, %s",
			 ifp->name, state->group->id, inet_ntoa(astate->addr));
	if (ia == NULL) {
		if (ifp->ctx->options & DHCPCD_TEST)
			goto test;
		ia = ipv4_addaddr(ifp, &astate->addr,
		    &inaddr_mask, &inaddr_bcast);
	}
	if (ia == NULL)
		return;
#ifdef IN_IFF_NOTREADY
	if (ia->addr_flags & IN_IFF_NOTREADY)
		return;
	logdebugx("%s: DAD completed for %s",
	    ifp->name, inet_ntoa(astate->addr));
#endif
test:
	state->addr = ia;
	if (ifp->ctx->options & DHCPCD_TEST) {
		script_runreason(ifp, "TEST");
		eloop_exit(ifp->ctx->eloop, EXIT_SUCCESS);
		return;
	}
	timespecclear(&state->defend);
	if_initrt(ifp->ctx, AF_INET);
	rt_build(ifp->ctx, AF_INET);
	arp_announce(astate);
	script_runreason(ifp, "IPV4LL");
	dhcpcd_daemonise(ifp->ctx);
	ipv4llg_writeclaims(ifp->ctx);
}

static void
ipv4llg_announced(struct arp_state *astate)
{
	struct ipv4llg_state *state = astate->cookie;

	state->conflicts = 0;
	/* Need to keep the arp state so we can defend our IP. */
}

static void
ipv4llg_probe(void *arg)
{

#ifdef IN_IFF_TENTATIVE
	ipv4llg_probed(arg);
#else
	arp_probe(arg);
#endif
}

static void
ipv4llg_conflicted(struct arp_state *astate, const struct arp_msg *amsg)
{
	struct interface *ifp;
	struct ipv4llg_state *state;
#ifdef IN_IFF_DUPLICATED
	struct ipv4_addr *ia;
#endif

	assert(astate != NULL);
	assert(astate->iface != NULL);
	ifp = astate->iface;
	state = astate->cookie;
	assert(state != NULL);

	/*
	 * NULL amsg means kernel detected DAD.
	 * We always fail on matching sip.
	 * We only fail on matching tip and we haven't added that address yet.
	 */
	if (amsg == NULL ||
	    amsg->sip.s_addr == astate->addr.s_addr ||
	    (amsg->sip.s_addr == 0 && amsg->tip.s_addr == astate->addr.s_addr
	     && ipv4_iffindaddr(ifp, &amsg->tip, NULL) == NULL))
		astate->failed = astate->addr;
	else
		return;

	arp_report_conflicted(astate, amsg);

	if (state->addr != NULL &&
	    astate->failed.s_addr == state->addr->addr.s_addr)
	{
#ifdef KERNEL_RFC5227
		logwarnx("%s: IPv4LL defence failed for %s",
		    ifp->name, state->addr->saddr);
#else
		struct timespec now, defend;

		/* RFC 3927 Section 2.5 says a defence should
		 * broadcast an ARP announcement.
		 * Because the kernel will also unicast a reply to the
		 * hardware address which requested the IP address
		 * the other IPv4LL client will receieve two ARP
		 * messages.
		 * If another conflict happens within DEFEND_INTERVAL
		 * then we must drop our address and negotiate a new one. */
		defend.tv_sec = state->defend.tv_sec + DEFEND_INTERVAL;
		defend.tv_nsec = state->defend.tv_nsec;
		clock_gettime(CLOCK_MONOTONIC, &now);
		if (timespeccmp(&defend, &now, >))
			logwarnx("%s: IPv4LL %d second defence failed for %s",
			    ifp->name, DEFEND_INTERVAL, state->addr->saddr);
		else if (arp_request(ifp,
		    state->addr->addr.s_addr, state->addr->addr.s_addr) == -1)
			logerr(__func__);
		else {
			logdebugx("%s: defended IPv4LL address %s",
			    ifp->name, state->addr->saddr);
			state->defend = now;
			return;
		}
#endif
		ipv4_deladdr(state->addr, 1);
		state->down = 1;
		state->addr = NULL;
		script_runreason(ifp, "IPV4LLG");
	}

#ifdef IN_IFF_DUPLICATED
	ia = ipv4_iffindaddr(ifp, &astate->addr, NULL);
	if (ia != NULL && ia->addr_flags & IN_IFF_DUPLICATED)
		ipv4_deladdr(ia, 1);
#endif

	arp_cancel(astate);
	if (++state->conflicts == MAX_CONFLICTS)
		logerr("%s: failed to acquire an IPv4LL address",
		    ifp->name);
	astate->addr.s_addr = ipv4llg_pickaddr(astate);
	loginfox("%s: probing for an IPv4LLg address: group %s, %s", ifp->name, state->group->id, inet_ntoa(astate->addr));
	eloop_timeout_add_sec(ifp->ctx->eloop,
		state->conflicts >= MAX_CONFLICTS ?
		RATE_LIMIT_INTERVAL : PROBE_WAIT,
		ipv4llg_probe, astate);
}

static void
ipv4llg_arpfree(struct arp_state *astate)
{
	struct ipv4llg_state *state;

	state = astate->cookie;
	if (state->arp == astate)
		state->arp = NULL;
}

static void
ipv4llg_start_one(struct interface *ifp, struct ipv4ll_group *group, struct ipv4llg_state *state)
{
	struct arp_state *astate;
	struct ipv4_addr *ia;

	loginfox("%s: starting up IPv4LLg: group %s, %s/%d", ifp->name, group->id,
	         inet_ntoa(group->range_begin), inet_ntocidr(group->network));

	if (state->arp != NULL)
		return;

	/* RFC 3927 Section 2.1 states that the random number generator
	 * SHOULD be seeded with a value derived from persistent information
	 * such as the IEEE 802 MAC address so that it usually picks
	 * the same address without persistent storage. */
	if (state->conflicts == 0) {
		unsigned int seed;
		char *orig;

		if (sizeof(seed) > ifp->hwlen) {
			seed = 0;
			memcpy(&seed, ifp->hwaddr, ifp->hwlen);
		} else
			memcpy(&seed, ifp->hwaddr + ifp->hwlen - sizeof(seed),
			    sizeof(seed));
		/* coverity[dont_call] */
		orig = initstate(seed,
		    state->randomstate, sizeof(state->randomstate));

		/* Save the original state. */
		if (ifp->ctx->randomstate == NULL)
			ifp->ctx->randomstate = orig;

		/* Set back the original state until we need the seeded one. */
		setstate(ifp->ctx->randomstate);
	}

	if ((astate = arp_new(ifp, NULL)) == NULL)
		return;

	state->group = group;
	state->arp = astate;
	astate->always_announce = 1;
	astate->cookie = state;
	astate->probed_cb = ipv4llg_probed;
	astate->announced_cb = ipv4llg_announced;
	astate->conflicted_cb = ipv4llg_conflicted;
	astate->free_cb = ipv4llg_arpfree;

	/* Find an existing group address and ensure we can work with it. */
	ia = NULL;
	{
		struct ipv4_state *ifstate = IPV4_STATE(ifp);
		TAILQ_FOREACH(ia, &ifstate->addrs, next) {
			if (IN_LINKLOCAL_GROUP(ntohl(ia->addr.s_addr), group))
				break;
		}
	}
	if (ia)
		logdebugx("%s: already have address %s", ifp->name, ia->saddr);

#ifdef IN_IFF_TENTATIVE
	if (ia != NULL && ia->addr_flags & IN_IFF_DUPLICATED) {
		ipv4_deladdr(ia, 0);
		ia = NULL;
	}
#endif

	if (ia != NULL) {
		astate->addr = ia->addr;
#ifdef IN_IFF_TENTATIVE
		if (ia->addr_flags & (IN_IFF_TENTATIVE | IN_IFF_DETACHED)) {
			loginfox("%s: waiting for DAD to complete on %s",
			    ifp->name, inet_ntoa(ia->addr));
			return;
		}
		loginfox("%s: using IPv4LLg address: %s", ifp->name, ia->saddr);
#endif
		ipv4llg_probed(astate);
		return;
	}

	astate->addr.s_addr = ipv4llg_pickaddr(astate);
	loginfox("%s: probing for an IPv4LLg address: group %s, %s", ifp->name, group->id, inet_ntoa(astate->addr));
#ifdef IN_IFF_TENTATIVE
	ipv4llg_probed(astate);
#else
	arp_probe(astate);
#endif
}

static void
ipv4llg_read_saved_claim(struct interface *ifp, ssize_t read, char *line)
{
	struct ipv4ll_group *group;

	const ssize_t cmp_buf_size = 2048;
	char cmp_buf[cmp_buf_size];

	TAILQ_FOREACH(group, &ifp->options->ipv4ll_groups, next) {
		ssize_t written;
		ssize_t cmp_buf_free = cmp_buf_size;
		ssize_t cmp_buf_used = 0;

		/* Check if the group matches... */
		written = snprintf(cmp_buf, cmp_buf_size, "%s %s ", group->id, inet_ntoa(group->range_begin));
		if (written >= cmp_buf_size)
			continue;

		cmp_buf_used += written;
		cmp_buf_free -= written;
		written = snprintf(cmp_buf + written, cmp_buf_free, "%s %d ", inet_ntoa(group->range_end),
		                   inet_ntocidr(group->network));
		if (written >= cmp_buf_free)
			continue;

		cmp_buf_used += written;
		cmp_buf_free -= written;

		if (read > cmp_buf_used &&
		    strncmp(line, cmp_buf, cmp_buf_used) == 0)
		{
			/* The group matches, so just parse the claim address now... */
			char *addr = line + cmp_buf_used;

			/* Clean up any newlines etc. */
			addr[strcspn(addr, "\r\n")] = 0;

			if (inet_aton(addr, &group->last_address))
			{
				loginfox("%s: using previously claimed address %s for IPv4LLg group %s", ifp->name, addr, group->id);
			} else
			{
				loginfox("%s: unable to parse previously claimed address for IPv4LLg group %s", ifp->name, addr);
			}
		}
	}
}

void
ipv4llg_start(void *arg)
{
	struct interface *ifp;
	struct ipv4llg_state *state;
	struct ipv4ll_group *group;
	size_t idx = 0;

	assert(arg != NULL);
	ifp = arg;
	if (ifp->if_data[IF_DATA_IPV4LLG] == NULL) {
		ifp->if_data[IF_DATA_IPV4LLG] = calloc(ifp->options->ipv4ll_groups_len, sizeof(*state));
		if (ifp->if_data[IF_DATA_IPV4LLG] == NULL) {
			logerr(__func__);
			return;
		}
	}

	/* Parse a stored claims list, for preferred addresses */
	/* Format: id range_begin range_end mask preferred_address */
	if (ifp->options->ipv4ll_groups_claimsfile)
	{
		char *line = NULL;
		size_t len = 0;
		ssize_t read;

		FILE *f = fopen(ifp->options->ipv4ll_groups_claimsfile, "r");
		if (f != NULL)
		{
			while ((read = getline(&line, &len, f)) != -1) {
				ipv4llg_read_saved_claim(ifp, read, line);
			}

			free(line);
			fclose(f);
		}
	}

	TAILQ_FOREACH(group, &ifp->options->ipv4ll_groups, next) {
		ipv4llg_start_one(ifp, group, IPV4LLG_STATE(ifp, idx++));
	}
}

int
ipv4llg_freedrop_one(struct interface *ifp, struct ipv4llg_state *state, int drop)
{
	int dropped;
	dropped = 0;

	/* Free ARP state first because ipv4_deladdr might also ... */
	if (state && state->arp) {
		eloop_timeout_delete(ifp->ctx->eloop, NULL, state->arp);
		arp_free(state->arp);
		state->arp = NULL;
	}

	if (drop && (ifp->options->options & DHCPCD_NODROP) != DHCPCD_NODROP) {
		if (state && state->addr != NULL) {
			ipv4_deladdr(state->addr, 1);
			state->addr = NULL;
			dropped = 1;
		}
	}

	return dropped;
}

void
ipv4llg_freedrop(struct interface *ifp, int drop)
{
	struct ipv4ll_group *group;
	size_t idx = 0;
	unsigned int dropped = 0;

	assert(ifp != NULL);
	if (!ifp->if_data[IF_DATA_IPV4LLG])
		return;

	TAILQ_FOREACH(group, &ifp->options->ipv4ll_groups, next) {
		dropped |= ipv4llg_freedrop_one(ifp, IPV4LLG_STATE(ifp, idx++), drop);
	}

	free(ifp->if_data[IF_DATA_IPV4LLG]);
	ifp->if_data[IF_DATA_IPV4LLG] = NULL;

	if (dropped) {
		rt_build(ifp->ctx, AF_INET);
		script_runreason(ifp, "IPV4LLG");
	}
}

/* This may cause issues in BSD systems, where running as a single dhcpcd
 * daemon would solve this issue easily. */
#ifdef HAVE_ROUTE_METRIC
int
ipv4llg_recvrt(__unused int cmd, const struct rt *rt)
{
	struct dhcpcd_ctx *ctx;
	struct interface *ifp;

	/* Ignore route init. */
	if (rt->rt_dflags & RTDF_INIT)
		return 0;

	/* Only interested in default route changes. */
	if (sa_is_unspecified(&rt->rt_dest))
		return 0;

	/* If any interface is running IPv4LL, rebuild our routing table. */
	ctx = rt->rt_ifp->ctx;
	TAILQ_FOREACH(ifp, ctx->ifaces, next) {
		if (ifp->if_data[IF_DATA_IPV4LLG] != NULL) {
			size_t idx = 0;
			struct ipv4ll_group *group;

			TAILQ_FOREACH(group, &ifp->options->ipv4ll_groups, next) {
				if (IPV4LLG_STATE_RUNNING(ifp, idx++)) {
					if_initrt(ctx, AF_INET);
					rt_build(ctx, AF_INET);
					break;
				}
			}
		}
	}

	return 0;
}
#endif

void
ipv4llg_writeclaims(void *arg)
{
	struct dhcpcd_ctx *ctx;
	struct interface *ifp;

	assert(arg != NULL);
	ctx = arg;

	TAILQ_FOREACH(ifp, ctx->ifaces, next) {
		if (!ifp->options || !ifp->options->ipv4ll_groups_claimsfile)
			continue;

		if (ifp->if_data[IF_DATA_IPV4LLG] != NULL) {
			size_t idx = 0;
			struct ipv4ll_group *group;

			char path[PATH_MAX];
			int fd, ret;

			if (snprintf(path, PATH_MAX, "%s.XXXXXX", ifp->options->ipv4ll_groups_claimsfile) >= PATH_MAX)
			{
				logerr("%s: failed to open claimsfile for writing, too long", ifp->name);
				goto out;
			}

			fd = mkstemp(path);
			if (fd == -1)
			{
				logerr("%s: opening temporary file failed: %s", ifp->name, strerror(errno));
				goto out;
			}

			TAILQ_FOREACH(group, &ifp->options->ipv4ll_groups, next) {
				struct ipv4llg_state *state = IPV4LLG_STATE(ifp, idx++);
				if (state->addr) {
					ret = dprintf(fd, "%s %s ", group->id, inet_ntoa(group->range_begin));
					if (ret < 0)
						goto out_open;
					ret = dprintf(fd, "%s ", inet_ntoa(group->range_end));
					if (ret < 0)
						goto out_open;
					ret = dprintf(fd, "%d %s\n",
					              inet_ntocidr(group->network),
					              inet_ntoa(state->addr->addr));
					if (ret < 0)
						goto out_open;
				}
			}

			if (close(fd) == -1)
			{
				logerr("%s: closing claimsfile failed: %s", ifp->name, strerror(errno));
				goto out_open;
			}

			if (rename(path, ifp->options->ipv4ll_groups_claimsfile) == -1)
			{
				logerr("%s: renaming claimsfile failed: %s", ifp->name, strerror(errno));
				goto out_open;
			}

			/* All good... */
			continue;

			/* Failure cases... */
out_open:
			/* So stale temporary files aren't kept around... */
			unlink(path);
out:
			continue;
		}
	}
}

#endif
