
/*
 *	sockGen.c -- Posix Socket support module for general posix use
 *
 *	Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 */

/******************************** Description *********************************/

/*
 *	Posix Socket Module.  This supports blocking and non-blocking buffered 
 *	socket I/O.
 */

#if (!defined (WIN) || defined (LITTLEFOOT) || defined (WEBS))

/********************************** Includes **********************************/
#ifndef CE
#include	<errno.h>
#include	<fcntl.h>
#include	<string.h>
#include	<stdlib.h>
#endif

#include	"uemf.h"

#ifdef VXWORKS
	#include	<hostLib.h>
#endif

#ifdef JRD_FEATURE_SIMPLE_DNS
#include <pthread.h> 
#include <arpa/nameser.h>
#include <net/if.h>
#include <sys/ioctl.h>

  #ifdef JRD_CONNECTED_REDIRECT_GOAL_PAGE
#include	"webs.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include "string.h"
#include <netdb.h>
  #endif
#endif
#include	"wsIntrn.h"
#include <common/jrd_common_def.h>


/************************************ Locals **********************************/

extern socket_t		**socketList;			/* List of open sockets */
extern int			socketMax;				/* Maximum size of socket */
extern int			socketHighestFd;		/* Highest socket fd opened */
static int			socketOpenCount = 0;	/* Number of task using sockets */

/***************************** Forward Declarations ***************************/

static void socketAccept(socket_t *sp);
static int 	socketDoEvent(socket_t *sp);
static int	tryAlternateConnect(int sock, struct sockaddr *sockaddr);

typedef enum {
  E_WWAN_STATUS_MIN = 0,
  E_WWAN_DISCONNECTED = E_WWAN_STATUS_MIN,
  E_WWAN_CONNECTING,
  E_WWAN_CONNECTED,
  E_WWAN_DISCONNECTING,
  E_WWAN_STATUS_MAX,
} e_jrd_conn_status_t;

static e_jrd_conn_status_t g_conn_status = E_WWAN_DISCONNECTED;
static char *conn_redirect_page=NULL;
static char *conn_redirect_page_key=NULL;
char jrd_host_name[128]="mw40.home";

extern int if_support_connect_redirect(void);
extern int get_connect_redirect_page(void);
/*********************************** Code *************************************/
/*
 *	Open socket module
 */

int socketOpen()
{
#if (defined (CE) || defined (WIN))
    WSADATA 	wsaData;
#endif

	if (++socketOpenCount > 1) {
		return 0;
	}

#if (defined (CE) || defined (WIN))
	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
		return -1;
	}
	if (wsaData.wVersion != MAKEWORD(1,1)) {
		WSACleanup();
		return -1;
	}
#endif

	socketList = NULL;
	socketMax = 0;
	socketHighestFd = -1;

	return 0;
}

/******************************************************************************/
/*
 *	Close the socket module, by closing all open connections
 */

void socketClose()
{
	int		i;

	if (--socketOpenCount <= 0) {
		for (i = socketMax; i >= 0; i--) {
			if (socketList && socketList[i]) {
				socketCloseConnection(i);
			}
		}
		socketOpenCount = 0;
	}
}

/******************************************************************************/
/*
 *	Open a client or server socket. Host is NULL if we want server capability.
    fengzhou modified, not use Host to judge if it's server.
 */

int socketOpenConnection(char *host, int port, socketAccept_t accept, int flags, bool is_client)
{
#if (!defined (NO_GETHOSTBYNAME) && !defined (VXWORKS))
	struct hostent		*hostent;					/* Host database entry */
#endif /* ! (NO_GETHOSTBYNAME || VXWORKS) */
	socket_t			*sp;
	struct sockaddr_in	sockaddr;
	int					sid, bcast, dgram, rc;

	if (port > SOCKET_PORT_MAX) {
		return -1;
	}
/*
 *	Allocate a socket structure
 */
	if ((sid = socketAlloc(host, port, accept, flags)) < 0) {
		return -1;
	}
	sp = socketList[sid];
	a_assert(sp);

/*
 *	Create the socket address structure
 */
	memset((char *) &sockaddr, '\0', sizeof(struct sockaddr_in));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons((short) (port & 0xFFFF));

	if (host == NULL || 0 == gstrlen(host)) {
		sockaddr.sin_addr.s_addr = INADDR_ANY;
	} else {
		sockaddr.sin_addr.s_addr = inet_addr(host);
		if (sockaddr.sin_addr.s_addr == INADDR_NONE) {
/*
 *			If the OS does not support gethostbyname functionality, the macro:
 *			NO_GETHOSTBYNAME should be defined to skip the use of gethostbyname.
 *			Unfortunatly there is no easy way to recover, the following code
 *			simply uses the basicGetHost IP for the sockaddr.
 */

#ifdef NO_GETHOSTBYNAME
			if (strcmp(host, basicGetHost()) == 0) {
				sockaddr.sin_addr.s_addr = inet_addr(basicGetAddress());
			}
			if (sockaddr.sin_addr.s_addr == INADDR_NONE) {
				socketFree(sid);
				return -1;
			}
#elif (defined (VXWORKS))
			sockaddr.sin_addr.s_addr = (unsigned long) hostGetByName(host);
			if (sockaddr.sin_addr.s_addr == NULL) {
				errno = ENXIO;
				socketFree(sid);
				return -1;
			}
#else
			hostent = gethostbyname(host);
			if (hostent != NULL) {
				memcpy((char *) &sockaddr.sin_addr, 
					(char *) hostent->h_addr_list[0],
					(size_t) hostent->h_length);
			} else {
				char	*asciiAddress;
				char_t	*address;

				address = basicGetAddress();
				asciiAddress = ballocUniToAsc(address, gstrlen(address));
				sockaddr.sin_addr.s_addr = inet_addr(asciiAddress);
				bfree(B_L, asciiAddress);
				if (sockaddr.sin_addr.s_addr == INADDR_NONE) {
					errno = ENXIO;
					socketFree(sid);
					return -1;
				}
			}
#endif /* (NO_GETHOSTBYNAME || VXWORKS) */
		}
	}

	bcast = sp->flags & SOCKET_BROADCAST;
	if (bcast) {
		sp->flags |= SOCKET_DATAGRAM;
	}
	dgram = sp->flags & SOCKET_DATAGRAM;

/*
 *	Create the socket. Support for datagram sockets. Set the close on
 *	exec flag so children don't inherit the socket.
 */
	sp->sock = socket(AF_INET, dgram ? SOCK_DGRAM: SOCK_STREAM, 0);
	if (sp->sock < 0) {
		socketFree(sid);
		return -1;
	}
#ifndef __NO_FCNTL
	fcntl(sp->sock, F_SETFD, FD_CLOEXEC);
#endif
	socketHighestFd = max(socketHighestFd, sp->sock);

/*
 *	If broadcast, we need to turn on broadcast capability.
 */
	if (bcast) {
		int broadcastFlag = 1;
		if (setsockopt(sp->sock, SOL_SOCKET, SO_BROADCAST,
				(char *) &broadcastFlag, sizeof(broadcastFlag)) < 0) {
			socketFree(sid);
			return -1;
		}
	}

/*
 *	Host is set if we are the client
 */
	if (is_client) {
	  if(!host || !gstrlen(host))
	  {
	    JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"client address not be given!!!\n");
	    return -1;
	  }
/*
 *		Connect to the remote server in blocking mode, then go into 
 *		non-blocking mode if desired.
 */
		if (!dgram) {
			if (! (sp->flags & SOCKET_BLOCK)) {
/*
 *				sockGen.c is only used for Windows products when blocking
 *				connects are expected.  This applies to webserver connectws. 
 *				Therefore the asynchronous connect code here is not compiled.
 */
#if (defined (WIN) || defined (CE)) && (!defined (LITTLEFOOT) && !defined (WEBS))
				int flag;

				sp->flags |= SOCKET_ASYNC;
/*
 *				Set to non-blocking for an async connect
 */
				flag = 1;
				if (ioctlsocket(sp->sock, FIONBIO, &flag) == SOCKET_ERROR) {
					socketFree(sid);
					return -1;
				}
#else
				socketSetBlock(sid, 1);
#endif /* #if (WIN || CE) && !(LITTLEFOOT || WEBS) */

			}
			if ((rc = connect(sp->sock, (struct sockaddr *) &sockaddr,
				sizeof(sockaddr))) < 0 && 
				(rc = tryAlternateConnect(sp->sock,
				(struct sockaddr *) &sockaddr)) < 0) {
#if (defined (WIN) || defined (CE))
				if (socketGetError() != EWOULDBLOCK) {
					socketFree(sid);
					return -1;
				}
#else
				socketFree(sid);
				return -1;

#endif /* WIN || CE */

			}
		}
	} else {
/*
 *		Bind to the socket endpoint and the call listen() to start listening
 */
		rc = 1;
		setsockopt(sp->sock, SOL_SOCKET, SO_REUSEADDR, (char *)&rc, sizeof(rc));
		if(1)
		{
			struct ifreq interface;
			strncpy(interface.ifr_name, host_interface, IFNAMSIZ);
			setsockopt(sp->sock, SOL_SOCKET, SO_BINDTODEVICE,(char *)&interface, sizeof(interface));
		}
		if (bind(sp->sock, (struct sockaddr *) &sockaddr, 
				sizeof(sockaddr)) < 0) {
			socketFree(sid);
			return -1;
		}

		if (! dgram) {
			if (listen(sp->sock, SOMAXCONN) < 0) {
				socketFree(sid);
				return -1;
			}
			sp->flags |= SOCKET_LISTENING;
		}
		sp->handlerMask |= SOCKET_READABLE;
	}

/*
 *	Set the blocking mode
 */

	if (flags & SOCKET_BLOCK) {
		socketSetBlock(sid, 1);
	} else {
		socketSetBlock(sid, 0);
	}
	return sid;
}


/******************************************************************************/
/*
 *	If the connection failed, swap the first two bytes in the 
 *	sockaddr structure.  This is a kludge due to a change in
 *	VxWorks between versions 5.3 and 5.4, but we want the 
 *	product to run on either.
 */

static int tryAlternateConnect(int sock, struct sockaddr *sockaddr)
{
#ifdef VXWORKS
	char *ptr;

	ptr = (char *)sockaddr;
	*ptr = *(ptr+1);
	*(ptr+1) = 0;
	return connect(sock, sockaddr, sizeof(struct sockaddr));
#else
	return -1;
#endif /* VXWORKS */
}

/******************************************************************************/
/*
 *	Close a socket
 */

void socketCloseConnection(int sid)
{
	socket_t	*sp;

	if ((sp = socketPtr(sid)) == NULL) {
		return;
	}
	socketFree(sid);
}

/******************************************************************************/
/*
 *	Accept a connection. Called as a callback on incoming connection.
 */

static void socketAccept(socket_t *sp)
{
	struct sockaddr_in	addr;
	socket_t 			*nsp;
	size_t				len;
	char				*pString;
	int 				newSock, nid;


#ifdef NW
	NETINET_DEFINE_CONTEXT;
#endif

	a_assert(sp);

/*
 *	Accept the connection and prevent inheriting by children (F_SETFD)
 */
	len = sizeof(struct sockaddr_in);
	if ((newSock = 
		accept(sp->sock, (struct sockaddr *) &addr, (socklen_t *) &len)) < 0) {
		return;
	}
#ifndef __NO_FCNTL
	fcntl(newSock, F_SETFD, FD_CLOEXEC);
#endif
	socketHighestFd = max(socketHighestFd, newSock);

/*
 *	Create a socket structure and insert into the socket list
 */
	nid = socketAlloc(sp->host, sp->port, sp->accept, sp->flags);
	nsp = socketList[nid];
	a_assert(nsp);
	nsp->sock = newSock;
	nsp->flags &= ~SOCKET_LISTENING;

	if (nsp == NULL) {
		return;
	}
/*
 *	Set the blocking mode before calling the accept callback.
 */

	socketSetBlock(nid, (nsp->flags & SOCKET_BLOCK) ? 1: 0);
/*
 *	Call the user accept callback. The user must call socketCreateHandler
 *	to register for further events of interest.
 */
	if (sp->accept != NULL) {
		pString = inet_ntoa(addr.sin_addr);
		if ((sp->accept)(nid, pString, ntohs(addr.sin_port), sp->sid) < 0) {
			socketFree(nid);
		}
#ifdef VXWORKS
		free(pString);
#endif
	}
}

/******************************************************************************/
/*
 *	Get more input from the socket and return in buf.
 *	Returns 0 for EOF, -1 for errors and otherwise the number of bytes read.
 */

int socketGetInput(int sid, char *buf, int toRead, int *errCode)
{
	struct sockaddr_in 	server;
	socket_t			*sp;
	int 				len, bytesRead;

	a_assert(buf);
	a_assert(errCode);

	*errCode = 0;

	if ((sp = socketPtr(sid)) == NULL) {
		return -1;
	}

/*
 *	If we have previously seen an EOF condition, then just return
 */
	if (sp->flags & SOCKET_EOF) {
		return 0;
	}
#if ((defined (WIN) || defined (CE)) && (!defined (LITTLEFOOT) && !defined  (WEBS)))
	if ( !(sp->flags & SOCKET_BLOCK)
			&& ! socketWaitForEvent(sp,  FD_CONNECT, errCode)) {
		return -1;
	}
#endif

/*
 *	Read the data
 */
	if (sp->flags & SOCKET_DATAGRAM) {
		len = sizeof(server);
		bytesRead = recvfrom(sp->sock, buf, toRead, 0,
			(struct sockaddr *) &server, (socklen_t *) &len);
	} else {
		bytesRead = recv(sp->sock, buf, toRead, 0);
	}
   
   /*
    * BUG 01865 -- CPU utilization hangs on Windows. The original code used 
    * the 'errno' global variable, which is not set by the winsock functions
    * as it is under *nix platforms. We use the platform independent
    * socketGetError() function instead, which does handle Windows correctly. 
    * Other, *nix compatible platforms should work as well, since on those
    * platforms, socketGetError() just returns the value of errno.
    * Thanks to Jonathan Burgoyne for the fix.
    */
   if (bytesRead < 0) 
   {
      *errCode = socketGetError();
      if (*errCode == ECONNRESET) 
      {
         sp->flags |= SOCKET_CONNRESET;
         return 0;
      }
      return -1;
   }
	return bytesRead;
}

/******************************************************************************/
/*
 *	Process an event on the event queue
 */


/******************************************************************************/
/*
 *	Define the events of interest
 */

void socketRegisterInterest(socket_t *sp, int handlerMask)
{
	a_assert(sp);

	sp->handlerMask = handlerMask;
}

/******************************************************************************/
/*
 *	Wait until an event occurs on a socket. Return 1 on success, 0 on failure.
 *	or -1 on exception (UEMF only)
 */

int socketWaitForEvent(socket_t *sp, int handlerMask, int *errCode)
{
	int	mask;

	a_assert(sp);

	mask = sp->handlerMask;
	sp->handlerMask |= handlerMask;
	while (socketSelect(sp->sid, 1000)) {
		if (sp->currentEvents & (handlerMask | SOCKET_EXCEPTION)) {
			break;
		}
	}
	sp->handlerMask = mask;
	if (sp->currentEvents & SOCKET_EXCEPTION) {
		return -1;
	} else if (sp->currentEvents & handlerMask) {
		return 1;
	}
	if (errCode) {
		*errCode = errno = EWOULDBLOCK;
	}
	return 0;
}

/******************************************************************************/
/*
 *	Return TRUE if there is a socket with an event ready to process,
 */

int socketReady(int sid)
{
	socket_t 	*sp;
	int			all;

	all = 0;
	if (sid < 0) {
		sid = 0;
		all = 1;
	}


	//sometimes, the socketMax is 0, it can't be 0
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "proc... %d\n", socketMax);
	//for test
	if (socketMax <= 0)
	{
		system("echo socketMax_is_zero > /tmp/log/sockzero.txt");
		socketMax = 1;  											// it must be at least 1
	}
	//test end
	

	for (; sid < socketMax; sid++) {
		if ((sp = socketList[sid]) == NULL) {
			if (! all) {
				break;
			} else {
				continue;
			}
		} 
		if (sp->flags & SOCKET_CONNRESET) {
			socketCloseConnection(sid);
			return 0;
		}
		if (sp->currentEvents & sp->handlerMask) {
			return 1;
		}
/*
 *		If there is input data, also call select to test for new events
 */
		if (sp->handlerMask & SOCKET_READABLE && socketInputBuffered(sid) > 0) {
			socketSelect(sid, 0);
			return 1;
		}
		if (! all) {
			break;
		}
	}
	return 0;
}

/******************************************************************************/
/*
 * 	Wait for a handle to become readable or writable and return a number of 
 *	noticed events. Timeout is in milliseconds.
 */

#if (defined (WIN) || defined (CE) || defined (NW))

int socketSelect(int sid, int timeout)
{
	struct timeval	tv;
	socket_t		*sp;
	fd_set		 	readFds, writeFds, exceptFds;
	int 			nEvents;
	int				all, socketHighestFd;	/* Highest socket fd opened */

	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&exceptFds);
	socketHighestFd = -1;

	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

/*
 *	Set the select event masks for events to watch
 */
	all = nEvents = 0;

	if (sid < 0) {
		all++;
		sid = 0;
	}

	for (; sid < socketMax; sid++) {
		if ((sp = socketList[sid]) == NULL) {
			continue;
		}
		a_assert(sp);
/*
 * 		Set the appropriate bit in the ready masks for the sp->sock.
 */
		if (sp->handlerMask & SOCKET_READABLE) {
			FD_SET(sp->sock, &readFds);
			nEvents++;
			if (socketInputBuffered(sid) > 0) {
				tv.tv_sec = 0;
				tv.tv_usec = 0;
			}
		}
		if (sp->handlerMask & SOCKET_WRITABLE) {
			FD_SET(sp->sock, &writeFds);
			nEvents++;
		}
		if (sp->handlerMask & SOCKET_EXCEPTION) {
			FD_SET(sp->sock, &exceptFds);
			nEvents++;
		}
		if (! all) {
			break;
		}
	}

/*
 *	Windows select() fails if no descriptors are set, instead of just sleeping
 *	like other, nice select() calls. So, if WIN, sleep.
 */
	if (nEvents == 0) {
		Sleep(timeout);
		return 0;
	}

/*
 * 	Wait for the event or a timeout.
 */
	nEvents = select(socketHighestFd+1, &readFds, &writeFds, &exceptFds, &tv);

	if (all) {
		sid = 0;
	}
	for (; sid < socketMax; sid++) {
		if ((sp = socketList[sid]) == NULL) {
			continue;
		}

		if (FD_ISSET(sp->sock, &readFds) || socketInputBuffered(sid) > 0) {
				sp->currentEvents |= SOCKET_READABLE;
		}
		if (FD_ISSET(sp->sock, &writeFds)) {
				sp->currentEvents |= SOCKET_WRITABLE;
		}
		if (FD_ISSET(sp->sock, &exceptFds)) {
				sp->currentEvents |= SOCKET_EXCEPTION;
		}
		if (! all) {
			break;
		}
	}

	return nEvents;
}

#else /* not WIN || CE || NW */

int socketSelect(int sid, int timeout)
{
	socket_t		*sp;
	struct timeval	tv;
	fd_mask 		*readFds, *writeFds, *exceptFds;
	int 			all, len, nwords, index, bit, nEvents;

/*
 *	Allocate and zero the select masks
 */
	nwords = (socketHighestFd + NFDBITS) / NFDBITS;
	len = nwords * sizeof(fd_mask);

	readFds = balloc(B_L, len);
	memset(readFds, 0, len);
	writeFds = balloc(B_L, len);
	memset(writeFds, 0, len);
	exceptFds = balloc(B_L, len);
	memset(exceptFds, 0, len);

	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

/*
 *	Set the select event masks for events to watch
 */
	all = nEvents = 0;

	if (sid < 0) {
		all++;
		sid = 0;
	}

	for (; sid < socketMax; sid++) {
		if ((sp = socketList[sid]) == NULL) {
			if (all == 0) {
				break;
			} else {
				continue;
			}
		}
		a_assert(sp);

/*
 * 		Initialize the ready masks and compute the mask offsets.
 */
		index = sp->sock / (NBBY * sizeof(fd_mask));
		bit = 1 << (sp->sock % (NBBY * sizeof(fd_mask)));
		
/*
 * 		Set the appropriate bit in the ready masks for the sp->sock.
 */
		if (sp->handlerMask & SOCKET_READABLE) {
			readFds[index] |= bit;
			nEvents++;
			if (socketInputBuffered(sid) > 0) {
				tv.tv_sec = 0;
				tv.tv_usec = 0;
			}
		}
		if (sp->handlerMask & SOCKET_WRITABLE) {
			writeFds[index] |= bit;
			nEvents++;
		}
		if (sp->handlerMask & SOCKET_EXCEPTION) {
			exceptFds[index] |= bit;
			nEvents++;
		}
		if (! all) {
			break;
		}
	}

/*
 * 	Wait for the event or a timeout. Reset nEvents to be the number of actual
 *	events now.
 */
	nEvents = select(socketHighestFd + 1, (fd_set *) readFds,
		(fd_set *) writeFds, (fd_set *) exceptFds, &tv);

	if (nEvents > 0) {
		if (all) {
			sid = 0;
		}
		for (; sid < socketMax; sid++) {
			if ((sp = socketList[sid]) == NULL) {
				if (all == 0) {
					break;
				} else {
					continue;
				}
			}

			index = sp->sock / (NBBY * sizeof(fd_mask));
			bit = 1 << (sp->sock % (NBBY * sizeof(fd_mask)));

			if (readFds[index] & bit || socketInputBuffered(sid) > 0) {
				sp->currentEvents |= SOCKET_READABLE;
			}
			if (writeFds[index] & bit) {
				sp->currentEvents |= SOCKET_WRITABLE;
			}
			if (exceptFds[index] & bit) {
				sp->currentEvents |= SOCKET_EXCEPTION;
			}
			if (! all) {
				break;
			}
		}
	}

	bfree(B_L, readFds);
	bfree(B_L, writeFds);
	bfree(B_L, exceptFds);

	return nEvents;
}
#endif /* WIN || CE */

/******************************************************************************/
/*
 *	Process socket events
 */

void socketProcess(int sid)
{
	socket_t	*sp;
	int			all;

	all = 0;
	if (sid < 0) {
		all = 1;
		sid = 0;
	}
/*
 * 	Process each socket
 */
	for (; sid < socketMax; sid++) {
		if ((sp = socketList[sid]) == NULL) {
			if (! all) {
				break;
			} else {
				continue;
			}
		}
		if (socketReady(sid)) {
			socketDoEvent(sp);
		}
		if (! all) {
			break;
		}
	}
}

/******************************************************************************/
/*
 *	Process an event on the event queue
 */

static int socketDoEvent(socket_t *sp)
{
	ringq_t		*rq;
	int 		sid;

	a_assert(sp);

	sid = sp->sid;
	if (sp->currentEvents & SOCKET_READABLE) {
		if (sp->flags & SOCKET_LISTENING) { 
			socketAccept(sp);
			sp->currentEvents = 0;
			return 1;
		} 

	} else {
/*
 *		If there is still read data in the buffers, trigger the read handler
 *		NOTE: this may busy spin if the read handler doesn't read the data
 */
		if (sp->handlerMask & SOCKET_READABLE && socketInputBuffered(sid) > 0) {
			sp->currentEvents |= SOCKET_READABLE;
		}
	}


/*
 *	If now writable and flushing in the background, continue flushing
 */
	if (sp->currentEvents & SOCKET_WRITABLE) {
		if (sp->flags & SOCKET_FLUSHING) {
			rq = &sp->outBuf;
			if (ringqLen(rq) > 0) {
				socketFlush(sp->sid);
			} else {
				sp->flags &= ~SOCKET_FLUSHING;
			}
		}
	}

/*
 *	Now invoke the users socket handler. NOTE: the handler may delete the
 *	socket, so we must be very careful after calling the handler.
 */
	if (sp->handler && (sp->handlerMask & sp->currentEvents)) {
		(sp->handler)(sid, sp->handlerMask & sp->currentEvents, 
			sp->handler_data);
/*
 *		Make sure socket pointer is still valid, then reset the currentEvents.
 */ 
		if (socketList && sid < socketMax && socketList[sid] == sp) {
			sp->currentEvents = 0;
		}
	}
	return 1;
}

/******************************************************************************/
/*
 *	Set the socket blocking mode
 */

int socketSetBlock(int sid, int on)
{
	socket_t		*sp;
	unsigned long	flag;
	int				iflag;
	int				oldBlock;

	flag = iflag = !on;

	if ((sp = socketPtr(sid)) == NULL) {
		a_assert(0);
		return 0;
	}
	oldBlock = (sp->flags & SOCKET_BLOCK);
	sp->flags &= ~(SOCKET_BLOCK);
	if (on) {
		sp->flags |= SOCKET_BLOCK;
	}

/*
 *	Put the socket into block / non-blocking mode
 */
	if (sp->flags & SOCKET_BLOCK) {
#if (defined (CE) || defined (WIN))
		ioctlsocket(sp->sock, FIONBIO, &flag);
#elif (defined (ECOS))
		int off;
		off = 0;
		ioctl(sp->sock, FIONBIO, &off);
#elif (defined (VXWORKS) || defined (NW))
		ioctl(sp->sock, FIONBIO, (int)&iflag);
#else
		fcntl(sp->sock, F_SETFL, fcntl(sp->sock, F_GETFL) & ~O_NONBLOCK);
#endif

	} else {
#if (defined (CE) || defined (WIN))
		ioctlsocket(sp->sock, FIONBIO, &flag);
#elif (defined (ECOS))
		int on;
		on = 1;
		ioctl(sp->sock, FIONBIO, &on);
#elif (defined (VXWORKS) || defined (NW))
		ioctl(sp->sock, FIONBIO, (int)&iflag);
#else
		fcntl(sp->sock, F_SETFL, fcntl(sp->sock, F_GETFL) | O_NONBLOCK);
#endif
	}
	/* Prevent SIGPIPE when writing to closed socket on OS X */
#ifdef MACOSX
	iflag = 1;
	setsockopt(sp->sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&iflag, sizeof(iflag));
#endif
	return oldBlock;
}

/******************************************************************************/
/*
 *	Return true if a readable socket has buffered data. - not public
 */

int socketDontBlock()
{
	socket_t	*sp;
	int			i;

	for (i = 0; i < socketMax; i++) {
		if ((sp = socketList[i]) == NULL || 
				(sp->handlerMask & SOCKET_READABLE) == 0) {
			continue;
		}
		if (socketInputBuffered(i) > 0) {
			return 1;
		}
	}
	return 0;
}

/******************************************************************************/
/*
 *	Return true if a particular socket buffered data. - not public
 */

int socketSockBuffered(int sock)
{
	socket_t	*sp;
	int			i;

	for (i = 0; i < socketMax; i++) {
		if ((sp = socketList[i]) == NULL || sp->sock != sock) {
			continue;
		}
		return socketInputBuffered(i);
	}
	return 0;
}

#endif /* (!WIN) | LITTLEFOOT | WEBS */

#ifdef JRD_FEATURE_SIMPLE_DNS
#define JRD_SIMPLE_DNS_PORT 54
#define JRD_DNS_BUF_SIZE 256

#ifdef JRD_CONNECTED_REDIRECT_GOAL_PAGE
static int jrd_dns_tim;
#endif
static char *dn_or_ip = NULL;
#ifdef JRD_DNS_DBG
static char *ly_pString;
#endif
static int jrd_dns_fd;
static fd_set jrd_dns_rset;
static struct in_addr jrd_answer_ip;
unsigned char jrd_dns_buf[JRD_DNS_BUF_SIZE];
static int fix_fd(int fd)
{
  int flags;

  if ((flags = fcntl(fd, F_GETFL)) == -1 ||
      fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    return -1;
  
  return 0;
}

static int jrd_dns_parse_host_from_query(unsigned char * dns_query_buf, unsigned char * dns_host_parse, unsigned short* dns_seg_count )
{
  unsigned short dns_seg_count_;
  unsigned char dns_query_buf_;
  unsigned short icount_buf = 0;
  unsigned short icount_host_char = 0;
  
  while( icount_buf < JRD_DNS_BUF_SIZE ) {
  	unsigned short icount_char = 0;
	unsigned short ichar = 0;
	icount_char = dns_query_buf[icount_buf];
	printf("=======================>dns_seg_count:%d,icount_char:%d\n", *dns_seg_count, icount_char);
	if ( icount_char >= 1 ) {
	    for( ichar = 1; ichar <= icount_char; ichar ++ ) {
	        dns_host_parse[icount_host_char] = dns_query_buf[icount_buf + ichar];
		 icount_host_char ++;
	    }
	    dns_host_parse[icount_host_char ++] = '.';
	    *dns_seg_count ++;
	    icount_buf = icount_buf + icount_char;
	} else {
	    icount_host_char --;
	    dns_host_parse[icount_host_char] = 0;
	    break;
	}
	icount_buf ++;
  }
 
  return 0;
}

/******************************************************************************/
/*
 *	Get the host info
 */

int jrd_get_host_info(char *host_ip, char*host_mask)
{
  struct sockaddr_in *my_ip;
  struct sockaddr_in *addr;
  struct sockaddr_in myip;
  my_ip = &myip;
  struct ifreq ifr;
  int sock;
  char *hostIp=NULL,*hostMask=NULL;
  if(!host_ip && !host_mask)
  {
    return -1;
  }
  if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    return -1;
  }
  strcpy(ifr.ifr_name, host_interface);
  if(host_ip)
  {
    if(ioctl(sock, SIOCGIFADDR, &ifr) < 0)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"get host ip ioctl error!");
	  close(sock);
      return -1;
    }
    my_ip->sin_addr = ((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr;
    hostIp = inet_ntoa(my_ip->sin_addr);
    memcpy(host_ip, hostIp, gstrlen(hostIp));
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"host ip: %s\n",host_ip);
  }
  if(host_mask)
  {
    if( ioctl( sock, SIOCGIFNETMASK, &ifr) == -1 )
	{
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"get host mask ioctl error!");
	  close(sock);
      return -1;
    }
    addr = (struct sockaddr_in *) & (ifr.ifr_addr);
    hostMask = inet_ntoa( addr->sin_addr);
    memcpy(host_mask, hostMask, gstrlen(hostMask));
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"host mask: %s\n",host_mask);
  }


  JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "get host info, sock:%d\n", sock);
  close(sock);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "get host info end\n");
  	
  return 0;
}


static int jrd_get_gateway_ip(struct in_addr* ip_addr)
{
  struct ifreq ifr;
  struct sockaddr_in* sock_addr;
#ifdef JRD_DNS_DBG
  struct in_addr test_addr;
#endif
  static struct in_addr g_ip_addr = {0};

  if(ip_addr == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_get_gateway_ip invalid argument\n");
    return -1;
  }
  if(g_ip_addr.s_addr)
  {
    ip_addr->s_addr = g_ip_addr.s_addr;
    return 0;
  }
//  strncpy(ifr.ifr_name, "rndis0", IF_NAMESIZE);
  strncpy(ifr.ifr_name, "bridge0", IF_NAMESIZE);
  ifr.ifr_addr.sa_family = AF_INET;
  if(ioctl(jrd_dns_fd, SIOCGIFADDR, &ifr) != -1)
  {
    sock_addr = (struct sockaddr_in*)&(ifr.ifr_addr);
	#ifdef JRD_DNS_DBG
    ly_pString = inet_ntoa(test_addr);
	#endif
    if(sock_addr->sin_addr.s_addr != 0)
    {
      g_ip_addr.s_addr = ip_addr->s_addr = sock_addr->sin_addr.s_addr;
	  #ifdef JRD_DNS_DBG
      ly_pString = inet_ntoa(*ip_addr);
    
	  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_get_answer_ip success eth0 %s\n", ly_pString);
	  #endif
      return 0;
    }
  }
  
  strncpy(ifr.ifr_name, "ecm0", IF_NAMESIZE);
  ifr.ifr_addr.sa_family = AF_INET;
  if(ioctl(jrd_dns_fd, SIOCGIFADDR, &ifr) != -1)
  {
    sock_addr = (struct sockaddr_in*)&(ifr.ifr_addr);
	#ifdef JRD_DNS_DBG
    ly_pString = inet_ntoa(test_addr);
	#endif
    if(sock_addr->sin_addr.s_addr != 0)
    {
      g_ip_addr.s_addr = ip_addr->s_addr = sock_addr->sin_addr.s_addr;
	  #ifdef JRD_DNS_DBG
      ly_pString = inet_ntoa(*ip_addr);

	  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_get_answer_ip success eth0 %s\n", ly_pString);
	  #endif
      return 0;
    }
  }
  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_dns_build_answer SIOCGIFADDR failed\n");
  return -1;

}


static int jrd_get_answer_ip(struct in_addr* ip_addr)
{
  struct ifreq ifr;
  struct sockaddr_in* sock_addr;
  #ifdef JRD_DNS_DBG
  struct in_addr test_addr;
  #endif

  if(ip_addr == NULL)
  {
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_get_answer_ip invalid argument\n");
    return -1;
  }

  if(if_support_connect_redirect()&&conn_redirect_page&&E_WWAN_CONNECTED == g_conn_status)/*if configur connect redirect and network connected*/
  {
    struct hostent *host;
    char *dn_or_ip = "192.168.1.1";
    char dns_buf[128];
    int ibuf=0;
    unsigned char dns_host_parse[128];
    unsigned short dns_seg_count = 0;
    unsigned short icount_retry_for_tim = 0;
    //char REDIRECT_PAGE[]= "timinternet";
    FILE *stream = NULL;
  
    //printf("=========================================>jrd_dns_buf=%s\n",jrd_dns_buf+13);
    memset(dns_buf, 0, 64);
    memcpy(dns_buf, &jrd_dns_buf[12], 128);
    memset(dns_host_parse, 0, 128);
    //strcpy(jrd_host_name, "y900.home");
    //dns_buf = &jrd_dns_buf[13];
    jrd_dns_parse_host_from_query( &jrd_dns_buf[12], dns_host_parse, &dns_seg_count);
    printf("=====================>dns_host_parse=%s\n",dns_host_parse);
    jrd_get_hostname(jrd_host_name);
    // for each domain name except host, added by Min, 2015/6/12
    if (strcmp(dns_host_parse, jrd_host_name)) {
      host = gethostbyname(dns_host_parse);
      if (host == NULL)
      {
        printf("get ip addr : ERROR !! ==========================>invalide dn(%s)\n", dns_host_parse);
        ip_addr->s_addr = 0;
        return -1;   
      }
      //dn_or_ip = inet_ntoa(*((struct in_addr *)host->h_addr));
      //printf("============================================>dns: %s, ip_addr(%s)\n", dns_host_parse, dn_or_ip);
      //inet_aton(dn_or_ip, ip_addr);
      ip_addr->s_addr = *(unsigned int*)host->h_addr;
     }
     else
     {
        return jrd_get_gateway_ip(ip_addr);
     }
    
       
    if(NULL != strstr(dns_host_parse, conn_redirect_page_key))  // for modified by Min, for TIM connected redirect, 2015/6/6
    {
      stream = popen( "/jrd-resource/bin/del_connect_redirect_rules.sh", "w" );
      if(!stream)
      {
        printf("%s failed to open stream for del_redirect_rules\n", __func__);
      }
      else
      {
         if( 0 > pclose( stream ) )
         {
           printf("%s failed to close stream for del_redirect_rules\n", __func__);
         }
      } 
    }
    return 0;
  }
  
  return jrd_get_gateway_ip(ip_addr);
}

static int jrd_dns_rcv_query(struct sockaddr_in*	p_src_addr)
{
  int ret = -1;
  /* Used for receive msg */
  HEADER *header = (HEADER *)jrd_dns_buf;
  struct iovec iov[1];
  struct msghdr msg;
  struct cmsghdr *cmptr;
  struct in_pktinfo *pack;
  union {
    struct cmsghdr align; /* this ensures alignment */
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
  } control_u;
  ssize_t n;
  int i = 0;
  struct timeval timeout;

  timeout.tv_sec = timeout.tv_usec = 0;
  memset(jrd_dns_buf, 0, JRD_DNS_BUF_SIZE);
  /* DNS query read session */
  //printf("jrd_dns_rcv_query start\n");
  if (select(jrd_dns_fd+1, &jrd_dns_rset, NULL, NULL, &timeout) < 0)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_dns_rcv_query select failed\n");
    FD_ZERO(&jrd_dns_rset);
  }

  if (FD_ISSET(jrd_dns_fd, &jrd_dns_rset))
  {
    //printf("jrd_dns_rcv_query jrd_dns_fd is set\n");

    iov[0].iov_base = jrd_dns_buf;
    iov[0].iov_len = JRD_DNS_BUF_SIZE;
      
    msg.msg_control = control_u.control;
    msg.msg_controllen = 0;
    msg.msg_flags = 0;
    msg.msg_name = p_src_addr;
    msg.msg_namelen = sizeof(struct sockaddr_in);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    if ((n = recvmsg(jrd_dns_fd, &msg, 0)) == -1)
    {
      ret = -1;

	  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_dns_rcv_query recvmsg for query msg failed\n");
      return ret;
    }

#ifdef JRD_CONNECTED_REDIRECT_GOAL_PAGE

char REDIRECT_PAGE[]= "csdn";
char* temp_buf = NULL;
 FILE *stream = NULL;

JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_dns_buf=%s\n",jrd_dns_buf+13);

      temp_buf = &jrd_dns_buf[13];
  if(NULL != strstr(temp_buf, REDIRECT_PAGE))
  {
  	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_is_redirect_page\n");
	jrd_dns_tim = 1;
	stream = popen( "/jrd-resource/bin/del_redirect_rules.sh", "w" );
	if(!stream)
	{
	  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s failed to open stream for del_redirect_rules\n", __func__);
	}
	else
	{
	   if( 0 > pclose( stream ) )
	   {
	     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s failed to close stream for del_redirect_rules\n", __func__);
	   }
	} 

  }
#endif

    #ifdef JRD_DNS_DBG
    if (n < (int)sizeof(HEADER) || 
        (msg.msg_flags & MSG_TRUNC) ||
        header->qr)
    {
      ret = -1;

	  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_dns_rcv_query recvmsg error for query len: %d, flag: 0x%02x\n", n, msg.msg_flags);
      return ret;
    }
    if (msg.msg_controllen < sizeof(struct cmsghdr))
    {
      ret = -1;

JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_dns_rcv_query recvmsg error for query msg_controllen: %d, size cmsghdr: %d\n", msg.msg_controllen, sizeof(struct cmsghdr));
      //return ret;
    }
    #endif
    ret = 0;
  }
  #ifdef JRD_DNS_DBG
  if(n > 0)
  {

	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd received dns query data start:\n");
    for(i = 0; i < n; i++)
 
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "0x%02X ", jrd_dns_buf[i]);
 
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "\njrd received dns query data end\n");
  }
  #endif

  //printf("jrd_dns_rcv_query end\n");
  return ret;
  /* DNS query read session */
}

static int jrd_dns_build_answer(HEADER *header, unsigned short* answer_len, struct in_addr* answer_addr)
{
  unsigned int nameoffset;
  unsigned char *p;
  int qtype, qclass;
  struct in_addr addr;

#ifdef JRD_CONNECTED_REDIRECT_GOAL_PAGE	
  struct hostent *host;
  char *dn_or_ip;

if(jrd_dns_tim == 1)
{
JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_dns_get_ip\n");
dn_or_ip=JRD_REDIRECT_PAGE;

       host = gethostbyname(dn_or_ip);
    if (host == NULL)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "get ip addr : ERROR !! invalide dn(%s)\n", dn_or_ip);
      return 0;
    }
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "get ip addr : DN(%s) -> ", dn_or_ip);
    dn_or_ip = inet_ntoa(*((struct in_addr *)host->h_addr));
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "ip_addr(%s)\n", dn_or_ip);

	inet_aton(dn_or_ip, &addr);
	
//inet_aton("184.168.221.31", &addr);
}
else
{
#endif

  if(answer_addr->s_addr == 0)
    inet_aton("192.168.1.1", &addr);
  else
    addr = *answer_addr;

#ifdef JRD_CONNECTED_REDIRECT_GOAL_PAGE 
}
#endif

  p = (unsigned char *)(header+1);

  nameoffset = p - (unsigned char *)header; 
  while(*p)
  {
    #ifdef JRD_DNS_DBG

	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_dns_build_answer: *p: 0x%02x ", *p);
	#endif
    p += *p;
    p++;
	#ifdef JRD_DNS_DBG

	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "after *p: 0x%02x\n", *p);
	#endif
  }
  p++; /* String terminal*/
  GETSHORT(qtype, p); 
  GETSHORT(qclass, p);
  #ifdef JRD_DNS_DBG

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_dns_build_answer: qtype: %d,  qclass: %d\n", qtype, qclass);
  #endif
  PUTSHORT(nameoffset | 0xc000, p);
  PUTSHORT(qtype, p);
  PUTSHORT(qclass, p);
  PUTLONG(0, p);      /* TTL */
  PUTSHORT(4, p);
  #ifdef JRD_DNS_DBG

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_dns_build_answer getsockname name: %s\n", ly_pString);
  #endif
  memcpy(p, &addr, INADDRSZ);
  p += INADDRSZ;
  header->qr = 1; /* response */
  header->aa = 0; /* authoritive - only hosts and DHCP derived names. */
  header->ra = 1; /* recursion if available */
  header->tc = 0; /* truncation */
  header->rcode = NOERROR; /* no error */
  header->ancount = htons(1);
  header->nscount = htons(0);
  header->arcount = htons(0);

  *answer_len = p -(unsigned char *)header;

  return 0;
}

static int jrd_dns_send_answer(void* data, unsigned short data_len, struct sockaddr_in*	to, struct in_addr* answer_addr)
{
  struct iovec iov[1];
  struct msghdr msg;
  struct cmsghdr *cmptr;
  union {
    struct cmsghdr align; /* this ensures alignment */
    char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
  } control_u;
  struct in_pktinfo pack;
  int retry_count = 5;
  struct in_addr addr;

  if(answer_addr->s_addr == 0)
    inet_aton("192.168.1.1", &addr);
  else
    addr = *answer_addr;
  /* Send session */
  iov[0].iov_base = (void*)data;
  iov[0].iov_len = data_len;

  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;
  msg.msg_name = to;
  msg.msg_namelen = sizeof(struct sockaddr_in);
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
#if 0
  msg.msg_control = control_u.control;
  msg.msg_controllen = sizeof(control_u);
  cmptr = CMSG_FIRSTHDR(&msg);
  pack.ipi_ifindex = 0;
  pack.ipi_spec_dst = addr;
  memcpy(CMSG_DATA(cmptr), &pack, sizeof(pack));
  msg.msg_controllen = cmptr->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
  cmptr->cmsg_level = SOL_IP;
  cmptr->cmsg_type = IP_PKTINFO;
#endif
retry:
  if (sendmsg(jrd_dns_fd, &msg, 0) == -1)
  {
    /* certain Linux kernels seem to object to setting the source address in the IPv6 stack
    by returning EINVAL from sendmsg. In that case, try again without setting the
    source address, since it will nearly alway be correct anyway.  IPv6 stinks. */
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd_dns_send_answer send answer failed remain errno: %d, retry: %d\n", errno, retry_count);
    if(retry_count-- > 0)
    {
      goto retry;
    }
    //if (retry_send() && retry_count == 0)
      //goto retry;
  }
}


static void config_redirect_rule(e_jrd_conn_status_t connect_status)
{
  switch( connect_status )
  {
    case E_WWAN_DISCONNECTED:
    {
      if(if_support_connect_redirect()&&conn_redirect_page)
      {
        system( "/jrd-resource/bin/del_connect_redirect_rules.sh" );
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "jrd del_connect_redirect_rules  \n");
      }

      system( "/jrd-resource/bin/add_redirect_rules.sh");
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "jrd add redirect_rules_proc \n");
    }
    break;
    case E_WWAN_CONNECTED:
    {
      system("/jrd-resource/bin/del_redirect_rules.sh");
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "jrd del redirect_rules_proc  \n");

      if(if_support_connect_redirect()&&conn_redirect_page)
      {
        char timinternet_ip_cmd[128];
        struct hostent *host;
        if(!dn_or_ip)
        {
          host = gethostbyname(conn_redirect_page);
          
          if (host != NULL)
          {
             dn_or_ip = strdup(inet_ntoa(*((struct in_addr *)host->h_addr)));
             memset(timinternet_ip_cmd, 0, 128);
             sprintf(timinternet_ip_cmd, "echo '%s' > /etc/timinternet_ip", dn_or_ip);
             system(timinternet_ip_cmd);
          }
          else
          {
            return;
          }
        }
        system( "/jrd-resource/bin/add_connect_redirect_rules.sh");
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "jrd add_connect_redirect_rules  \n");
      }
    }
    break;
    default:
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "invalid connec status,do no thing\n");
      return;
    }
    break;
  }
    
}

static void jrd_conn_status_ind_cb(void  *rx_buf, uint32 rx_buf_len)
{
  e_jrd_conn_status_t *conn_status = (e_jrd_conn_status_t *)rx_buf;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "conn status ind,pre connect status:%d,connect status:%d\n",g_conn_status,*conn_status);
  if(g_conn_status != *conn_status)
  {
    g_conn_status = *conn_status;
    config_redirect_rule(g_conn_status);
  }
}


static void *jrd_simple_dns_proc(void *handle)
{
  struct sockaddr_in	sockaddr;
  unsigned short answer_len = 0;
  HEADER *header = (HEADER *)jrd_dns_buf;
  int ret = 0;
  int i = 0;
  int count = 0;
  struct in_addr answer_addr;

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "simple dns start proc\n");
  memset(&answer_addr, 0, sizeof(struct in_addr));
  FD_ZERO(&jrd_dns_rset);
  
  if(!jrd_get_gateway_ip(&answer_addr)&&if_support_connect_redirect())
  {
    char *gateway_ip=NULL;
    char cmd[128]={0};
    gateway_ip = strdup(inet_ntoa(answer_addr));
    snprintf(cmd, sizeof(cmd), "echo '%s' > /etc/gateway_ip", gateway_ip);
    system(cmd);
  }
  while(ret = jrd_GetConnStatus(&g_conn_status))
  {
    count ++;

    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "get connect status failed,ret=%d,count=%d\n",ret,count);
    if(count > 3)
      break;
  }
  if(!ret)
  {
    config_redirect_rule(g_conn_status);
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "get connect status failed,ret=%d\n",ret);
  }
  
  //jrd_get_answer_ip(&answer_addr);
  jrd_get_gateway_ip(&answer_addr);
  while(1)
  {
    //if(answer_addr.s_addr == 0)
    //{
      //jrd_get_answer_ip(&answer_addr);
    //}
    if(if_support_connect_redirect()&&!dn_or_ip && E_WWAN_CONNECTED==g_conn_status)
    {
      config_redirect_rule(g_conn_status);
    }
    FD_SET(jrd_dns_fd, &jrd_dns_rset);    
    ret = jrd_dns_rcv_query(&sockaddr);
    if(ret)
    {
      sleep(1);
	  //printf("jrd_simple_dns_proc continue\n");
      continue;
    }
    FD_CLR(jrd_dns_fd, &jrd_dns_rset);
    //if(1)
    jrd_get_answer_ip(&answer_addr);
    
    jrd_dns_build_answer(header, &answer_len, &answer_addr);
	#ifdef JRD_DNS_DBG
    if(answer_len > 0)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd send dns answer data start:\n");
      
      for(i = 0; i < answer_len; i++)
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "0x%02X ", jrd_dns_buf[i]);

      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "\njrd send dns answer data end\n");
    }
    #endif
    jrd_dns_send_answer((void*)header, answer_len, &sockaddr, &answer_addr);    
  }

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd_simple_dns_proc end\n");

  return NULL;
}

int jrdInitSimpleDNS(void)
{
  int ret = 0;
  int opt = 1;
  struct sockaddr_in	sockaddr;
  pthread_t pthrd_id;

  if(if_support_connect_redirect())
  {
    struct in_addr answer_addr={0};
    conn_redirect_page = get_connect_redirect_page();
    if(conn_redirect_page&&strlen(conn_redirect_page))
    {
      if(!strncmp("www.",conn_redirect_page,strlen("www.")))
      {
        conn_redirect_page_key = conn_redirect_page+strlen("www.");
      }
      else
      {
        conn_redirect_page_key = conn_redirect_page;
      }
    }
    else
    {
      conn_redirect_page_key = conn_redirect_page = NULL;
    }
    
    if (jrd_get_hostname(jrd_host_name) == -1)
    {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd Get host name failed\n");
    }
    else
    {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "jrd Get host name: %s\n", jrd_host_name);
    }
    
    if(!jrd_get_gateway_ip(&answer_addr))
    {
      char *gateway_ip=NULL;
      char cmd[128]={0};
      gateway_ip = strdup(inet_ntoa(answer_addr));
      snprintf(cmd, sizeof(cmd), "echo '%s' > /etc/gateway_ip", gateway_ip);
      system(cmd);
    }
    
  }

  client_reg_indication_cb(IND_TO_CLT_CONN_STATE,jrd_conn_status_ind_cb);
  
  memset((char *) &sockaddr, '\0', sizeof(struct sockaddr_in));
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons((short) (JRD_SIMPLE_DNS_PORT & 0xFFFF));
  sockaddr.sin_addr.s_addr = INADDR_ANY;

  jrd_dns_fd = socket(sockaddr.sin_family, SOCK_DGRAM, 0);
  if(jrd_dns_fd == -1)
  {
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrdInitSimpleDNS create socket for DNS server failed\n");
    ret = -1;
    return ret;
  }
  ret = setsockopt(jrd_dns_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ;
  if(ret == -1)
  {
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrdInitSimpleDNS set socket OPT SO_REUSEADDR for DNS server failed\n");
    return ret;
  }
  //ret = fix_fd(jrd_dns_fd);
  if(ret == -1)
  {

	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrdInitSimpleDNS set socket fix_fd for DNS server failed\n");
    return ret;
  }
  ret = bind(jrd_dns_fd, &sockaddr, sizeof(sockaddr)) ;
  if(ret)
  {

	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrdInitSimpleDNS bind socket for DNS server failed\n");
    return ret;
  }
  /* DNS query read session */
  //FD_ZERO(&jrd_dns_rset);
  //FD_SET(jrd_dns_fd, &jrd_dns_rset);

  if( 0 != pthread_create( &pthrd_id,
                             NULL,
                             (void *) jrd_simple_dns_proc,
                             NULL ))
  {
    ret = -1;

	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrdInitSimpleDNS create dns process thread failed!\n");
  }  
  
  return ret;
}

#endif

/******************************************************************************/



























































































