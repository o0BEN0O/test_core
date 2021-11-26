/*
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/

/*
 *	Main program for for the GoAhead WebServer.
 */

/********************************* Includes ***********************************/

#include	"uemf.h"
#include	"wsIntrn.h"
#include	<signal.h>
#include	<unistd.h>
#include	<sys/types.h>

#ifdef WEBS_SSL_SUPPORT
#include	"websSSL.h"
#endif
#include "webs_define.h"
#include "sock_clt.h"
#ifdef USER_MANAGEMENT_SUPPORT
#include	"um.h"
void	formDefineUserMgmt(void);
#endif

#ifdef JRD_NEW_WEB_ARCH
//#define JRD_WEB_ROOT_PATH T("/jrd-resource/resource/webrc/www")
#define JRD_WEB_ROOT_PATH T("/ipq-resource/resource/webrc/www")

#endif
//#define JRD_WEB_ROOT_PATH T("www")
/*********************************** Locals ***********************************/
/*
 *	Change configuration here
 */
#ifdef JRD_NEW_WEB_ARCH
static char_t		*rootWeb = JRD_WEB_ROOT_PATH;			/* Root web directory */
#else
static char_t		*rootWeb = "www";			/* Root web directory */
#endif
static char_t		*demoWeb = T("wwwdemo");		/* Root web directory */
static char_t		*password = T("");				/* Security password */
static int			port = WEBS_DEFAULT_PORT;		/* Server port */
static int			retries = 5;					/* Server port retries */
static int			finished = 0;					/* Finished flag */

char_t host_interface[32]={0};

extern jrd_webs_pridata_t g_webs_data;


/****************************** Forward Declarations **************************/

static int 	initWebs(int demo);
static int	aspTest(int eid, webs_t wp, int argc, char_t **argv);
static void formTest(webs_t wp, char_t *path, char_t *query);
//static int aspTestJson(int eid, webs_t wp, int argc, char_t **argv);
static void formTestJson(webs_t wp, char_t * path, char_t * query);
static void formUploadFileTest(webs_t wp, char_t *path, char_t *query); // add by gyr 2011.10.15
static int  websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
				int arg, char_t *url, char_t *path, char_t *query);
static void	sigintHandler(int);
#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif
#if JRD_OEM_ULIMIT_OPEN
#include <sys/time.h>
#include <sys/resource.h>
#endif
#if JRD_OEM_ULIMIT_OPEN 
int set_ulimit()
{
    struct rlimit rlim = {0};
    printf("set_ulimit open and set ulimit\n");

    if (getrlimit(RLIMIT_CORE, &rlim)==0)
    {
        printf("getrlimit: %d, %d\n",rlim.rlim_cur, rlim.rlim_max);
        rlim.rlim_cur = rlim.rlim_max = RLIM_INFINITY; 
        if(setrlimit(RLIMIT_CORE, &rlim)!=0)
        {
            printf("setrlimit fail\n");
        }
    }
    else
    {
        printf("getrlimit fail\n");
    }
    return 0;
}
#endif

/*********************************** Code *************************************/
/*
 *	Main -- entry point from LINUX
 */
 #ifdef  JRD_REDIRECT_COMMON_MACRO
#define JRD_FILE_HOST_NAME    "/etc/hosts"
int jrd_get_hostname(char *hostname)
{
	FILE *f;
	char ip_str[32];
	char line[128];
	
JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd get_hostname\n");

    if(hostname == NULL)
    {

	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "you give me a NULL hostname\n");
    	return -1;
    }

	f = fopen(JRD_FILE_HOST_NAME, "r");
	/* Bypass the first line */
	fgets(line, sizeof(line), f);
	/* Read the host name that we want. */
	fgets(line, sizeof(line), f);
	
	if(sscanf(line, "%s %s\n",ip_str, hostname) != 2)
	{

	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "read host name from /etc/hosts failed\n");
	return -1;
	}
	fclose(f);
	
	return 0;
}
#endif
FILE *webs_log_fp=NULL;
extern int jrd_main_sock_response_flag;
static int jrd_udisk_log_init(char *process_name)
{
  char udisk_file_name[32];
  char jrd_log_flag[32];
  FILE *fd;
  char cmd[128]={0};

  fd = popen("echo /mnt/`ls -l /mnt/|head -1|awk '{print $9}'`/jrd_log|tr -d [\"\r\"][\"\n\"]", "r");
  if(fd != NULL)
  {
    if(NULL != fgets(jrd_log_flag,32,fd))
    {
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "get str successful,%s\n",jrd_log_flag);
       if (0 != access(jrd_log_flag,F_OK))
       {
         if(pclose(fd) != 0)
         {
           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "pclose fd failed\n");
         }

         return -1;
       }
    }
    else
    {
		if(pclose(fd) != 0)
		{
			JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "pclose fd failed\n");
		}
		return -1;
    }

  }
  else
  {
	return -1;
  }
  fd = NULL;
  snprintf(cmd,sizeof(cmd)-1,"echo /mnt/`ls -l /mnt/|head -1|awk '{print $9}'`/%s%d.log|tr -d [\"\r\"][\"\n\"]",process_name,getpid());
  fd = popen(cmd, "r");
  if(fd != NULL)
  {
    if(NULL != fgets(udisk_file_name,32,fd))
    {
       JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "get str successful,%s\n",udisk_file_name);
       webs_log_fp=fopen(udisk_file_name,"w+");
       if(pclose(fd) != 0)
       {
         JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "pclose fd failed\n");
       }
       return 0;
    }
    if(pclose(fd) != 0)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "pclose fd failed\n");
    }
  }  
  return -1;
}

int main(int argc, char** argv)
{
int i, demo = 0;  
jrd_udisk_log_init("webs");

webs_init_database();

#if JRD_OEM_ULIMIT_OPEN
	set_ulimit();
#endif
#ifdef JRD_WEBS_DONE_MUTEX
    pthread_mutex_init(&webs_done_handle_mutex, NULL);
#endif /*JRD_WEBS_DONE_MUTEX*/
  jrd_timer_init();
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-demo") == 0) {
			demo++;
		}
	}
	
/*
 *	Initialize the memory allocator. Allow use of malloc and start
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */
	bopen(NULL, (60 * 1024), B_USE_MALLOC);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigintHandler);
	signal(SIGTERM, sigintHandler);
/*
	boolean ret_val = Diag_LSM_Init(NULL);
	if ( !ret_val )
	{
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "webs main():  failed on DIAG init!\n");
	}
*/
#ifdef JRD_UNCONNECT_REDIRECT_REMEBER_LAST_URL  
    system("rm -rf /jrd-resource/resource/last_web_for_redirect");
    system("touch /jrd-resource/resource/last_web_for_redirect");
#endif  
#if 0
  //fengzhou add for push log to sd card
  if(access("/usr/oem/jrd_debug_push_log_to_sdcard_flag_file",0) == 0)
  {
    char sdcard_file_name[32];
    FILE *fd;
    #define SD_NAME "/dev/mmcblk0"
    fd = popen("cat /sys/devices/platform/msm_hsusb/gadget/lun0/file", "r");
    if(fd != NULL)
    {
      if(NULL != fgets(sdcard_file_name,32,fd))
      {
         JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "get str successful,%s\n",sdcard_file_name);
         if(!strncmp(sdcard_file_name, SD_NAME, strlen(SD_NAME)))
           webs_log_fp=fopen("/sdcard/webs_log.log","a+");
      }
      if(pclose(fd) != 0)
      {
         JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "pclose fd failed\n");
      }
    }
    if(webs_log_fp == NULL)
    {
        webs_log_fp=fopen("/data/webs.log","a+");
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "\n\n+++++++++++++++++++++++++++++++++webs log start here(fp=%p)+++++++++++++++++++++++++++++++++\n", webs_log_fp);
    }
  }
 //#else
   //if(access("/usr/oem/jrd_debug_push_log_to_sdcard_flag_file",0) == 0)
  //{


  //}
  
 #endif

  
//#ifndef JRD_FREE_ACCESS
  if (g_webs_data.free_access == 0)
  {
    EGOSM_Init();
    websInitSessionInfo();
  }
//#endif
/*
 *	Initialize the web server
 */
	if (initWebs(demo) < 0) {
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "initWebs error.\n");	// added by gyr 2011.09.17
		return -1;
	}
  start_sock_client(JRD_SERVER_PATH);

  //start_sock_test_thread();
  
#ifdef JRD_FEATURE_SIMPLE_DNS

    //jrdInitSimpleDNS();
#endif
#ifdef WEBS_SSL_SUPPORT
	websSSLOpen();
/*	websRequireSSL("/"); */	/* Require all files be served via https */
#endif

/*
 *	Basic event loop. SocketReady returns true when a socket is ready for
 *	service. SocketSelect will block until an event occurs. SocketProcess
 *	will actually do the servicing.
 */
	finished = 0;
	while (!finished) {
		if (socketReady(-1) || socketSelect(-1, 1000)) {
#ifdef JRD_WEBS_DONE_MUTEX
            pthread_mutex_lock(&webs_done_handle_mutex); 
#endif
			socketProcess(-1);
#ifdef JRD_WEBS_DONE_MUTEX
  		    pthread_mutex_unlock(&webs_done_handle_mutex); 
#endif
		}
		if(!jrd_main_sock_response_flag)
		  jrd_main_sock_response_flag = 1;
		websCgiCleanup();
#ifdef JRD_WEBS_DONE_MUTEX
        pthread_mutex_lock(&webs_done_handle_mutex);  
#endif
		emfSchedProcess();
#ifdef JRD_WEBS_DONE_MUTEX
        pthread_mutex_unlock(&webs_done_handle_mutex); 
#endif
	}    
#ifdef WEBS_SSL_SUPPORT
	websSSLClose();
#endif

#ifdef USER_MANAGEMENT_SUPPORT
	umClose();
#endif

/*
 *	Close the socket module, report memory leaks and close the memory allocator
 */
	websCloseServer();
	socketClose();
#ifdef B_STATS
	memLeaks();
#endif
	bclose();
#ifdef JRD_WEBS_DONE_MUTEX
    pthread_mutex_destroy(&webs_done_handle_mutex);
#endif
	return 0;
}

/*
 *	Exit cleanly on interrupt
 */
static void sigintHandler(int unused)
{
	finished = 1;
}

static void websSetHostInterface(void)
{
  FILE *fp = fopen(HOST_INTERFACE_CONF_FILE,"r");
  if(!fp)
  {
    gstrncpy(host_interface, "br-lan", sizeof(host_interface)-1);
  }
  else
  {
    fgets(host_interface, sizeof(host_interface)-1, fp);
    fclose(fp);

    if(host_interface[gstrlen(host_interface)-1]=='\n'||host_interface[gstrlen(host_interface)-1]=='\r')
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"remove \\n of host_interface: %s\n",host_interface);
      host_interface[gstrlen(host_interface)-1] = 0;
    }
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"host_interface: %s\n",host_interface);
  }
}

/******************************************************************************/
/*
 *	Initialize the web server.
 */

static int initWebs(int demo)
{
	//struct hostent	*hp;
	//struct in_addr	intaddr;
	char			host[128], dir[128], webdir[128],host_ip[64]={0};
	char			*cp;
	char_t			wbuf[128];

/*
 *	Initialize the socket subsystem
 */
	socketOpen();

#ifdef USER_MANAGEMENT_SUPPORT
/*
 *	Initialize the User Management database
 */
	umOpen();
	umRestore(T("umconfig.txt"));
#endif

/*
 *	Define the local Ip address, host name, default home page and the
 *	root web directory.
 */
 /* del by gyr 2011.09.17
	if (gethostname(host, sizeof(host)) < 0) {
		error(E_L, E_LOG, T("Can't get hostname"));
		printf("initWebs::Can't get hostname.\n");		// added by gyr 2011.09.17
		return -1;
	}
	if ((hp = gethostbyname(host)) == NULL) {
		error(E_L, E_LOG, T("Can't get host address"));
		printf("initWebs::Can't get hostname...\n");	// added by gyr 2011.09.17
		return -1;
	}
	memcpy((char *) &intaddr, (char *) hp->h_addr_list[0],
		(size_t) hp->h_length);
*/
	//intaddr.s_addr = inet_addr(T("192.168.1.1"));		// added by gyr 2011.09.17

/*
 *	Set ../web as the root web. Modify this to suit your needs
 *	A "-demo" option to the command line will set a webdemo root
 */
   #ifndef JRD_NEW_WEB_ARCH
	getcwd(dir, sizeof(dir));// Get the name of the current working directory, note | gyr 2011.09.17
	if ((cp = strrchr(dir, '/'))) {// 查找字符在指定字符串中从后面开始的第一次出现的位置，
		*cp = '\0';					// 如果成功，则返回指向该位置的指针，如果失败，则返回 false。
	}
	/*if (demo) {
		sprintf(webdir, "%s/%s", dir, demoWeb);
	} else {
		sprintf(webdir, "%s/%s", dir, rootWeb);
	}*/

    sprintf(webdir, "%s/%s", dir, rootWeb);
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "dir: %s, rootweb: %s, webdir: %s\n", dir, rootWeb, webdir);
    #else
    gstrcpy(webdir, JRD_WEB_ROOT_PATH);
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "web directory is: %s\n", webdir);
    #endif 

/*
 *	Configure the web server options before opening the web server
 */
	websSetHostInterface();
	websSetDefaultDir(webdir);
	//cp = inet_ntoa(intaddr);
	jrd_get_host_info(host_ip, NULL);
	cp = host_ip;
	ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
	websSetIpaddr(wbuf);
	if (g_webs_data.webs_unconnected_redirect == 1){
	#ifdef JRD_REDIRECT_COMMON_MACRO
	if (jrd_get_hostname(host) == -1)
	{
	 JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "jrd Get host name failed\n");
	}
	else
	{
	 JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "jrd Get host name: %s\n", host);
	}
	#endif
	}
	ascToUni(wbuf, host, min(strlen(host) + 1, sizeof(wbuf)));
	websSetHost(wbuf);

/*
 *	Configure the web server options before opening the web server
 */
	//websSetDefaultPage(T("index.asp"));
	/*modify by houailing 2013-03-09*/
	websSetDefaultPage(T("index.html"));
	websSetPassword(password);

/*
 *	Open the web server on the given port. If that port is taken, try
 *	the next sequential port for up to "retries" attempts.
 */
	websOpenServer(port, retries);

/*
 * 	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security
 *	handler, forms handler and the default web page handler.
 */
  // Connie modify start, 2014/12/15
	websUrlHandlerDefine(T(""),             NULL, 0, websSecurityHandler, WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T(""),             NULL, 0, websFormHandler,     WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T("/goform"),      NULL, 0, websJRDtnHandler,    0);
	websUrlHandlerDefine(T("/jrd/webapi"),  NULL, 0, websJRDJsonHandler,  0);
	websUrlHandlerDefine(T("/vodafoneapi"), NULL, 0, websJRDXMLHandler,   0);
	websUrlHandlerDefine(T("/cgi-bin"),     NULL, 0, websCgiHandler,      0);
	websUrlHandlerDefine(T(""),             NULL, 0, websDefaultHandler,  WEBS_HANDLER_LAST);
  // Connie modify end, 2014/12/15
/*
 *	Now define two test procedures. Replace these with your application
 *	relevant ASP script procedures and form functions.
 */
 /* 
   modify by hou ailing :2012-06-20

   decription: asp function regist

 */
    
	//websAspDefine(T("aspTest"), aspTest);
	//websAspDefine(T("aspTestJson"), aspTestJson);
	//websFormDefine(T("formTest"), formTest);
	//websFormDefine(T("formTestJson"), formTestJson);
	//websFormDefine(T("formUploadFileTest"), formUploadFileTest);// add by gyr 2011.10.15
	
/*end houailing*/

    webs_asp_init();


/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
	formDefineUserMgmt();
#endif

/*
 *	Create a handler for the default home page
 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0);

	return 0;
}

/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to
 *	"localhost/asp.asp" to test.
 */

static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;

	if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	return websWrite(wp, T("Name: %s, Address %s"), name, address);
}

/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

static void formTest(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T("Joe Smith"));
	address = websGetVar(wp, T("address"), T("1212 Milky Way Ave."));

	websHeader(wp);
	websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2></body>"), name, address);
	websFooter(wp);
	websDone(wp, 200);
}

/*test return json data*/
#if 0
static int aspTestJson(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;
	
	return websWrite(wp, T("{password:\"%s\",username:\"%s\",language:%s}"), webs_conf.password, webs_conf.username,webs_conf.language);
}
#endif
static void formTestJson(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T(""));
	address = websGetVar(wp, T("address"), T(""));
	
	websWrite(wp, T("HTTP/1.0 200 OK\n"));
	websWrite(wp, T("Pragma: no-cache\n"));
	websWrite(wp, T("Cache-control: no-cache\n"));
	websWrite(wp, T("Content-Type: text/html\n"));
	websWrite(wp, T("\n"));
	websWrite(wp, T("{\"Name\":\"%s\", \"Address\":\"%s\",\"intdata\":%d}"), name, address,6);
	websDone(wp, 200);
}
/*****************************************************************************/


/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
	int arg, char_t *url, char_t *path, char_t *query)
{
/*
 *	If the empty or "/" URL is invoked, redirect default URLs to the home page
 */
	if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
		websRedirect(wp, WEBS_DEFAULT_HOME);
		return 1;
	}
	return 0;
}

/******************************************************************************/

#ifdef B_STATS
static void memLeaks()
{
	int		fd;

	if ((fd = gopen(T("leak.txt"), O_CREAT | O_TRUNC | O_WRONLY, 0666)) >= 0) {
		bstats(fd, printMemStats);
		close(fd);
	}
}

/******************************************************************************/
/*
 *	Print memory usage / leaks
 */

static void printMemStats(int handle, char_t *fmt, ...)
{
	va_list		args;
	char_t		buf[256];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	write(handle, buf, strlen(buf));
}
#endif


/******************************************************************************/
/*
 * for test html upload file to web server
 * add by gyr 2011.10.15
 */

static void formUploadFileTest(webs_t wp, char_t *path, char_t *query)
{
    FILE *       fp;
    char_t *     fn;
    char_t *     bn = NULL;
    int          locWrite;
    int          numLeft;
    int          numWrite;

	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "\n...................formUploadFileTest...................\n\n");

    a_assert(websValid(wp));
    websHeader(wp);

    fn = websGetVar(wp, T("filename"), T(""));
    if (fn != NULL && *fn != '\0') {
        if ((int)(bn = gstrrchr(fn, '/') + 1) == 1) {
            if ((int)(bn = gstrrchr(fn, '\\') + 1) == 1) {
                bn = fn;
            }
        }
    }

	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "fn=%s, bn=%s  .......\n", fn, bn);

    websWrite(wp, T("Filename = %s<br>Size = %d bytes<br>"), bn, wp->lenPostData);

    if ((fp = fopen((bn == NULL ? "upldForm.bin" : bn), "w+b")) == NULL) {
        websWrite(wp, T("File open failed!<br>"));
    } else {
        locWrite = 0;
        numLeft = wp->lenPostData;
        while (numLeft > 0) {
            numWrite = fwrite(&(wp->postData[locWrite]), sizeof(*(wp->postData)), numLeft, fp);
            if (numWrite < numLeft) {
                websWrite(wp, T("File write failed.<br>  ferror=%d locWrite=%d numLeft=%d numWrite=%d Size=%d bytes<br>"), ferror(fp), locWrite, numLeft, numWrite, wp->lenPostData);
            break;
            }
            locWrite += numWrite;
            numLeft -= numWrite;
        }

        if (numLeft == 0) {
            if (fclose(fp) != 0) {
                websWrite(wp, T("File close failed.<br>  errno=%d locWrite=%d numLeft=%d numWrite=%d Size=%d bytes<br>"), errno, locWrite, numLeft, numWrite, wp->lenPostData);
            } else {
                websWrite(wp, T("File Size Written = %d bytes<br>"), wp->lenPostData);
            }
        } else {
            websWrite(wp, T("numLeft=%d locWrite=%d Size=%d bytes<br>"), numLeft, locWrite, wp->lenPostData);
        }
    }

    websFooter(wp);
    websDone(wp, 200);

}

/******************************************************************************/
