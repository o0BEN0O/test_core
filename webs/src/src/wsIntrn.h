/* 
 *	wsIntrn.h -- Internal GoAhead Web server header
 *
 * Copyright (c) GoAhead Software Inc., 1992-2010. All Rights Reserved.
 *
 *	See the file "license.txt" for information on usage and redistribution
 *
 */
 
#ifndef _h_WEBS_INTERNAL
#define _h_WEBS_INTERNAL 1

/******************************** Description *********************************/

/* 
 *	Internal GoAhead Web Server header. This defines the Web private APIs
 *	Include this header when you want to create URL handlers.
 */

/*********************************** Defines **********************************/

/********************************** Includes **********************************/

#include	<ctype.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#ifdef NETWARE
	#include	<fcntl.h>
	#include	<sys/stat.h>
	#include	<signal.h>
	#include	<io.h>
#endif

#ifdef WIN
	#include	<fcntl.h>
	#include	<sys/stat.h>
	#include	<io.h>
#define localtime_r(A, B)	localtime_s(B,A)
	#include	<share.h>
#define snprintf			_snprintf
#endif

#ifdef NW
	#include	<fcntl.h>
	#include	<sys/stat.h>
#endif

#ifdef SCOV5
	#include	<fcntl.h>
	#include	<sys/stat.h>
	#include	<signal.h>
	#include	<unistd.h>
#endif

#ifdef LYNX
	#include	<fcntl.h>
	#include	<sys/stat.h>
	#include	<signal.h>
	#include	<unistd.h>
#endif

#ifdef UNIX
	#include	<fcntl.h>
	#include	<sys/stat.h>
	#include	<signal.h>
	#include	<unistd.h>
#endif

#ifdef QNX4
	#include	<fcntl.h>
	#include	<sys/stat.h>
	#include	<signal.h>
	#include	<unistd.h>
	#include	<unix.h>
#endif

#ifdef UW
	#include	<fcntl.h>
	#include	<sys/stat.h>
#endif

#ifdef VXWORKS
	#include	<vxWorks.h>
	#include	<fcntl.h>
	#include	<sys/stat.h>
#endif

#ifdef SOLARIS
	#include	<macros.h>
	#include	<fcntl.h>
	#include	<sys/stat.h>
#endif
#ifdef JRD_NEW_WEB_ARCH
#include <stdarg.h>
#include <stdio.h>
//#include "msg.h"
#endif
#include	"uemf.h"
#include	"ejIntrn.h"
#include	"webs.h"
#include <common/jrd_common_def.h>

/********************************** Defines ***********************************/
/* 
 *	Read handler flags and state
 */
#define JRD_ATOI(str)                             atoi(str)
#define JRD_FCLOSE(file)                          fclose(file)  
    
#define JRD_MALLOC(sz,ptr) \
    do{  \
        (ptr)=malloc((sz));  \
        if(NULL!=(ptr))  \
        memset((ptr),0,(sz)); \
    }while(0)
    typedef enum
    {
       MODULE_INVALID = -1,
       MODULE_MIN,
       MODULE_MAIN = MODULE_MIN,
       MODULE_SOCK,
       MODULE_SMS,
       MODULE_USIM,
       MODULE_CONNECTION,
       MODULE_PROFILE,                 /*5*/
       MODULE_NETWORK,
       MODULE_WIFI,
       MODULE_ROUTER,
       MODULE_SYS,
       MODULE_MISC,                    /*10*/
       MODULE_SDSHARE, 
       MODULE_LCD, 
       MODULE_DEBUG,
       MODULE_PBM,
       MODULE_CHG,                     /*15*/
       MODULE_UI,
       MODULE_KEY,
       MODULE_TIME,
       MODULE_INPUT_LISTEN,
       MODULE_LED,                     /*20*/
       MODULE_FOTA,
       MODULE_USAGE,
       MODULE_SOFT_DOG,
     //Added by Tian Yiqing, Add cwmp module, Start.
#ifdef JRD_FEATURE_CWMP
       MODULE_CWMP,
#endif
     //Added by Tian Yiqing, Add cwmp module, End.
       MODULE_MAX,
    }module_enum_type;
    
#define JRD_OEM_SUB_MASK_SIZE 4    
typedef struct{
      module_enum_type module_id;
      uint16           act_id;
      uint32           sub_mask[JRD_OEM_SUB_MASK_SIZE]; 
    }web_act_info_type;
 
#define JRD_INVALID_SYM_FD -1
#define WEBS_BEGIN			0x1			/* Beginning state */
#define WEBS_HEADER			0x2			/* Ready to read first line */
#define WEBS_POST			0x4			/* POST without content */
#define WEBS_POST_CLEN		0x8			/* Ready to read content for POST */
#define WEBS_PROCESSING		0x10		/* Processing request */
#define WEBS_KEEP_TIMEOUT	15000		/* Keep-alive timeout (15 secs) */
//#define WEBS_TIMEOUT		60000		/* General request timeout (60) */
/*fengzhou modify to solve Slow HTTP Denial of Service Attack issue*/
#define WEBS_TIMEOUT		20000		/* General request timeout (20) */

#define PAGE_READ_BUFSIZE	512			/* bytes read from page files */
#define MAX_PORT_LEN		10			/* max digits in port number */
#define WEBS_SYM_INIT		64			/* initial # of sym table entries */
#define JRD_WEB_LOG_SIZE 512
/*
 *	URL handler structure. Stores the leading URL path and the handler
 *	function to call when the URL path is seen.
 */ 
typedef struct {
	int		(*handler)(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
			char_t *url, char_t *path, 
			char_t *query);					/* Callback URL handler function */
	char_t	*webDir;						/* Web directory if required */
	char_t	*urlPrefix;						/* URL leading prefix */
	int		len;							/* Length of urlPrefix for speed */
	int		arg;							/* Argument to provide to handler */
	int		flags;							/* Flags */
} websUrlHandlerType;

/* 
 *	Webs statistics
 */
typedef struct {
	long			errors;					/* General errors */
	long			redirects;
	long			net_requests;
	long			activeNetRequests;
	long			activeBrowserRequests;
	long 			timeouts;
	long			access;					/* Access violations */
	long 			localHits;
	long 			remoteHits;
	long 			formHits;
	long 			cgiHits;
	long 			handlerHits;
} websStatsType;

extern websStatsType websStats;				/* Web access stats */

/* 
 *	Error code list
 */
typedef struct {
	int		code;							/* HTTP error code */
	char_t	*msg;							/* HTTP error message */
} websErrorType;

/* 
 *	Mime type list
 */
typedef struct {
	char_t	*type;							/* Mime type */
	char_t	*ext;							/* File extension */
} websMimeType;

/*
 *	File information structure.
 */
typedef struct {
	unsigned long	size;					/* File length */
	int				isDir;					/* Set if directory */
	time_t			mtime;					/* Modified time */
} websStatType;

/*
 *	Compiled Rom Page Index
 */
typedef struct {
	char_t			*path;					/* Web page URL path */
	unsigned char	*page;					/* Web page data */
	int				size;					/* Size of web page in bytes */
	int				pos;					/* Current read position */
} websRomPageIndexType;

//Connie add start, 2016/4/11
typedef enum
{
  E_LOGIN_SUCCESS             = 0,
  E_LOGIN_FAIL                = 1,
  E_LOGIN_OTHER_HAS_LOGIN     = 2,
  E_LOGIN_FORCE_LOGIN_SUCCESS = 3,
}e_login_status_type;
//Connie add end, 2016/4/11

/*
 *	Defines for file open.
 */
#ifndef CE
#define	SOCKET_RDONLY	O_RDONLY
#define	SOCKET_BINARY	O_BINARY
#else /* CE */
#define	SOCKET_RDONLY	0x1
#define	SOCKET_BINARY	0x2
#endif /* CE */

extern websRomPageIndexType	websRomPageIndex[];
extern websMimeType		websMimeList[];		/* List of mime types */
extern sym_fd_t			websMime;			/* Set of mime types */
extern webs_t*			webs;				/* Session list head */
extern int				websMax;			/* List size */
extern char_t			websHost[64];		/* Name of this host */
extern char_t			websIpaddr[64];		/* IP address of this host */
extern char_t			*websHostUrl;		/* URL for this host */
extern char_t			*websIpaddrUrl;		/* URL for this host */
extern int				websPort;			/* Port number */

/******************************** Prototypes **********************************/

extern int		 websAspOpen();
extern void		 websAspClose();
extern void		 websFormOpen();
extern void		 websFormClose();
extern int		 websAspWrite(int ejid, webs_t wp, int argc, char_t **argv);

extern void  	 websDefaultOpen();
extern void  	 websDefaultClose();
#ifdef WEBS_WHITELIST_SUPPORT
#define WHITELIST_SSL       0x001   /* File only accessible through https */
#define WHITELIST_CGI       0x002   /* Node is in the cgi-bin dir */
extern int		websBuildWhitelist(void);
extern int		websWhitelistCheck(char *path);
extern void		websDeleteWhitelist(void);
#endif /* WEBS_WHITELIST_SUPPORT */
extern int 		 websDefaultHandler(webs_t wp, char_t *urlPrefix, 
					char_t *webDir, int arg, char_t *url, char_t *path, 
					char_t *query);
extern int 		 websFormHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, char_t *url, char_t *path, char_t *query);
extern int 		 websCgiHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
					int arg, char_t *url, char_t *path, char_t *query);
extern void		 websCgiCleanup();
extern int		 websCheckCgiProc(int handle);
extern char_t	 *websGetCgiCommName();

extern int		 websLaunchCgiProc(char_t *cgiPath, char_t **argp,
					char_t **envp, char_t *stdIn, char_t *stdOut);
extern int 		 websOpen(int sid);
extern void 	 websResponse(webs_t wp, int code, char_t *msg, 
					char_t *redirect);
extern int 		 websJavaScriptEval(webs_t wp, char_t *script);
extern int 		 websPageReadData(webs_t wp, char *buf, int nBytes);
extern int		 websPageOpen(webs_t wp, char_t *lpath, char_t *path, int mode,
					int perm);
extern void		 websPageClose(webs_t wp);
extern void		 websPageSeek(webs_t wp, long offset);
extern int 	 	 websPageStat(webs_t wp, char_t *lpath, char_t *path,
					websStatType *sbuf);
extern int		 websPageIsDirectory(char_t *lpath);
extern int 		 websRomOpen();
extern void		 websRomClose();
extern int 		 websRomPageOpen(webs_t wp, char_t *path, int mode, int perm);
extern void 	 websRomPageClose(int fd);
extern int 		 websRomPageReadData(webs_t wp, char *buf, int len);
extern int 	 	 websRomPageStat(char_t *path, websStatType *sbuf);
extern long		 websRomPageSeek(webs_t wp, long offset, int origin);
extern void 	 websSetRequestSocketHandler(webs_t wp, int mask, 
					void (*fn)(webs_t wp));
extern int 		 websSolutionHandler(webs_t wp, char_t *urlPrefix,
					char_t *webDir, int arg, char_t *url, char_t *path, 
					char_t *query);
extern void 	 websUrlHandlerClose();
extern int 		 websUrlHandlerOpen();
extern int 		 websOpenServer(int port, int retries);
extern void 	 websCloseServer();
extern char_t*	 websGetDateString(websStatType* sbuf);

extern int		strcmpci(char_t* s1, char_t* s2);

#ifdef CE
extern int writeUniToAsc(int fid, void *buf, unsigned int len);
extern int readAscToUni(int fid, void **buf, unsigned int len);
#endif
#ifdef JRD_NEW_WEB_ARCH
#define JRD_OEM_LOG_LOW     4 //MSG_LEGACY_HIGH   /*4*/
#define JRD_OEM_LOG_MEDIAM  8 //MSG_LEGACY_ERROR  /*8*/
#define JRD_OEM_LOG_HIGH    8 //MSG_LEGACY_ERROR  /*8*/
#define JRD_OEM_LOG_ERROR   16 //MSG_LEGACY_FATAL  /*16*/

#define HOST_INTERFACE_CONF_FILE "/etc/config/webs_host_interface"

extern char_t host_interface[32];

//#define JRD_PRINT_MSG( level, fmtString, x)                         \
        //MSG_SPRINTF_1(MSG_SSID_DIAG, level, fmtString, x) 

//#define JRD_OEM_LOG_SET_LEVEL JRD_OEM_LOG_LOW
extern int jrd_oem_log_level;

extern FILE *webs_log_fp;

#define JRD_OEM_LOG_INFO(level, x...)                    \
  do{ \
    if(level >= jrd_oem_log_level) \
    { \
        char sprint_buf[JRD_WEB_LOG_SIZE]; \
        printf("%s(%d) ", __FUNCTION__, __LINE__); \
        printf(x); \
        strcpy(sprint_buf, "webs: "); \
        jrd_printf(sprint_buf+6,JRD_WEB_LOG_SIZE-6,x); \
        if(1) \
        { \
          char jrd_str[512]; \
          snprintf(jrd_str,512,"%s(%d)%s",__FUNCTION__,__LINE__,sprint_buf); \
          jrd_wifi_save_log(jrd_str);\
        } \
    } \
  }while(0)

int jrd_printf(char* sprint_buf, int size, char *fmt, ...);
#endif

#endif /* _h_WEBS_INTERNAL */

/******************************************************************************/

