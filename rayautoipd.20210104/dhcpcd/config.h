/* linux */
#define SYSCONFDIR	"/usr/etc"
#define SBINDIR		"/usr/sbin"
#define LIBDIR		"/usr/lib"

#ifndef ANDROID
#define LIBEXECDIR	"/usr/libexec"
#define DBDIR		"/var/db/rayautoipd"
#define RUNDIR		"/var/run/rayautoipd"
#endif

#include		<asm/types.h> /* fix broken headers */
#include		<sys/socket.h> /* fix broken headers */
#include		<linux/rtnetlink.h>
#define HAVE_NL80211_H
// #define HAVE_IN6_ADDR_GEN_MODE_NONE
#include		"compat/arc4random.h"
#include		"compat/arc4random_uniform.h"
#include		"compat/strlcpy.h"
#include		"compat/pidfile.h"
#include		"compat/strtoi.h"
#include		<sys/queue.h>
#include		"compat/queue.h"
#include		"compat/reallocarray.h"
#define HAVE_REALLOCARRAY
#define HAVE_EPOLL
#include		"compat/endian.h"
#include		"compat/crypt/md5.h"
#include		"compat/crypt/sha256.h"
#include		"compat/crypt/hmac.h"

/* Android (Raymarine-specific) */
#ifdef ANDROID

#define NBBY CHAR_BIT
#include <net/if_ether.h>

#define LIBEXECDIR	"/system/bin"
#define DBDIR        "/mnt/tmp/rayautoipd"
#define RUNDIR       "/mnt/tmp/rayautoipd"
#define _PATH_VARRUN RUNDIR

#endif
