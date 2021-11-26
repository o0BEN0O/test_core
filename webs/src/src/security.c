/*
 * security.c -- Security handler
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/

/*
 *	This module provides a basic security policy.
 */

/********************************* Includes ***********************************/

#include	"wsIntrn.h"
#include	"um.h"
#ifdef DIGEST_ACCESS_SUPPORT
#include	"websda.h"
#endif
#include <json-c/json.h>
#include "webs_define.h"

/********************************** Defines ***********************************/
/*
 *	The following #defines change the behaviour of security in the absence 
 *	of User Management.
 *	Note that use of User management functions require prior calling of
 *	umInit() to behave correctly
 */

#ifndef USER_MANAGEMENT_SUPPORT
#define umGetAccessMethodForURL(url) AM_FULL
#define umUserExists(userid) 0
#define umUserCanAccessURL(userid, url) 1
#define umGetUserPassword(userid) websGetPassword()
#define umGetAccessLimitSecure(accessLimit) 0
#define umGetAccessLimit(url) NULL
#endif

/******************************** Local Data **********************************/

static char_t	websPassword[WEBS_MAX_PASS];	/* Access password (decoded) */
#ifdef _DEBUG
static int		debugSecurity = 1;
#else
static int		debugSecurity = 0;
#endif


/***begin**zlz added for session managerment by ip + ie type 2011-10-15**/
#define SESSION_FORMAT_IP 0
#define SESSION_FORMAT_IP_IETYPE 1

//#define SESSION_KEY_FORMAT  SESSION_FORMAT_IP
#define SESSION_KEY_FORMAT  SESSION_FORMAT_IP_IETYPE

/***begin**zlz added for session's heartbeat  2011-11-08**/
#define MAX_COUNT_FOR_DELETE_ITEM 32
#define MAX_LENGTH_OF_ITEMNAME 24
unsigned char g_delsessionids[MAX_COUNT_FOR_DELETE_ITEM][MAX_LENGTH_OF_ITEMNAME] = {0};
unsigned char g_delcounts = 0;
/***end**zlz added for session's heartbeat  2011-11-08**/

#define MAX_COUNT_OF_SESSION 256
#define INVALID_SYM_FD       -1
static sym_fd_t g_sessionsTable = INVALID_SYM_FD;

#define SESSIONID_FOR_LOGOUT  0
#define SESSIONID_FOR_LOGIN_FAILED  1
#define SESSIONID_FOR_HAS_LOGIN 2
#define SESSIONID_FOR_LOGIN_SUCCESS  5


#define STR_USERAGENT_OF_IE   "MSIE"
#define STR_USERAGENT_OF_CHROME   "Chrome"
#define STR_USERAGENT_OF_FIREFOX   "Firefox"
#define STR_USERAGENT_OF_SAFARI   "Safari"
#define STR_USERAGENT_OF_OPERA  "Opera"
#define STR_USERAGENT_OF_OPERA2  "OPR"
#define STR_USERAGENT_OF_EDGE  "Edge"


/***begin**zlz added for session's heartbeat  2011-11-08**/
#define SESSION_TIMEOUT_COUNT (6)

//session key format : ip_exploreTypeE, such as 192.168.1.234_1 
//in hash table, every item's  format : session-key = integer-number(session's id)
// integer-number 's high two bytes means session's timeout 
// integer-number 's low  two bytes means session's id
/***end**zlz added for session's heartbeat  2011-11-08**/
typedef enum
{
    eTYPE_UNKNOWN = 0,
    eTYPE_IE,
    eTYPE_CHROME,
    eTYPE_FIREFOX,
    eTYPE_SAFARI,
    eTYPE_OPERA,
    eTYPE_EDGE,
    eTYPE_MAX
 }exploreTypeE;

/***end**zlz added for session managerment by ip + ie type 2011-10-15**/


/***begin**zlz added for maintean user state 2011-08-18**/

#include <common/comdef.h>

#define STR_COOKIE_KEY "egoid"

#define MAX_SESSION_COUNT 256
unsigned char g_sessionids[MAX_SESSION_COUNT+4] = {0};
unsigned char g_cSessionPC = 0;


typedef struct{
    char accesslimitedname[32];
    int nlen;
}accessLimitedItemT;




accessLimitedItemT g_freeAccessDirs[] = {
    {"/images/", 0},
    {"/js/", 0},
    {"/css/", 0},
    {"/resource", 0},
    {"/help/", 0},
    {"/comm_help/", 0},
    {"/mobile/css/", 0},
    {"/mobile/js/", 0},
    {"/mobile/images/", 0},
#ifdef JRD_DEBUG_CATCH_LOG
    {"/logs/", 0},
#endif
#ifdef JRD_CUSTOM_APPS_API
    {"/vodafoneapi/",0},
    {"/orangeapi/",0},
#endif
    {"/files/",0},
    {"/app/",0},
    {"/lib/",0},
  /////////////////////add begin by wei.huang.sz/////////////////
    {"/html/",0},
    {"/usermanual/",0},
    {"/res/",0},
    {"/language/",0},
    {"/usermanual/",0},
  /////////////////////add end by wei.huang.sz/////////////////
    {"/dist/",0},//fengzhou added 170122
};

accessLimitedItemT g_freeAccessPages[] = {
	{"/index.asp", 0},
	{"/login.asp", 0},
	{"/comm_help.html", 0},
	{"/default.asp", 0},
	{"/gettestdata.asp", 0},
	{"/index.html", 0},//add for html page  houailing:2013-03-09
	{"/login.html", 0},
	{"/default.html", 0},
	{"/mobile/index.html", 0},
#ifdef JRD_DEBUG_CATCH_LOG
    {"/log/logconfig.html", 0},
#endif
	{"/faq.html", 0},
	{"/favicon.ico", 0},
	{"/connection/homePinManage.html", 0},
	{"/home.html", 0},
	{"/Legal_Mentions.html", 0},//add by wei.huang.sz for jason 2016-8-24
};

extern jrd_webs_pridata_t g_webs_data;
extern char * jrd_webs_token_str_extern;
extern char *jrd_default_token;

accessLimitedItemT g_freeAccessMethods[] = {
  /*{"GetLoginState", 0},
  {"HeartBeat", 0},
  {"GetUIPwState",0},
  {"SetUIPwState", 0},
  {"InitPwSecurity", 0},
  {"GetToken",0},
  //{"GetLoginToken", 0},
  {"LoginSecurity", 0},
  {"GetSimStatus", 0},
  {"UnlockPin", 0},
  {"UnlockPuk", 0},
  {"GetAutoValidatePinState", 0},
  {"SetAutoValidatePinState", 0},
  {"UnlockSimlock", 0},
  {"GetConnectionState", 0},
  {"GetConnectionSettings", 0},
  {"GetNetworkInfo", 0},
  //{"SearchNetwork", 0},
  {"SearchNetworkResult", 0},	  
  //{"RegisterNetwork", 0},
  {"GetNetworkRegisterState", 0},	  
  {"GetNetworkSettings", 0},	  
  {"GetWlanSettings", 0},	  
  {"getSmsInitState", 0},
  {"GetSMSContactList", 0},	  
  {"GetSMSContentList", 0},
  {"GetSMSStorageState", 0},	  
  {"GetSendSMSResult", 0},
  {"GetSingleSMS", 0},
  {"GetSMSListByContactNum", 0},
  {"GetDeviceNewVersion", 0},
  {"GetDeviceUpgradeState", 0},
  {"SetCheckNewVersion", 0},
  {"GetMacFilterSettings", 0},
  {"getIPFilterList", 0},
  {"getFirewallSwitch", 0},
  {"getDMZInfo", 0},
  {"GetLanSettings", 0},
  {"GetConnectedDeviceList", 0},
  {"GetBlockDeviceList", 0},
  {"GetSystemInfo", 0},
  {"SetLanguage", 0},
  {"GetCurrentLanguage", 0},
  {"GetSystemStatus", 0},
  {"GetProfileList", 0},
  {"GetPowerSavingMode", 0},
  {"GetUpnpSettings", 0},
  {"GetSMSSettings", 0},
  {"getSMSAutoRedirectSetting", 0},
  {"GetNewMessage", 0},
  {"GetWlanState", 0},
  {"getSMSStateByLocation", 0},
  {"getCurrentProfile",0},*/
  {"GetCurrentLanguage", 0},
  {"SetLanguage", 0},
  {"Login", 0},
  {"Logout", 0},
  {"GetLoginState", 0},
  {"GetSimStatus", 0},
  {"GetSystemStatus", 0},
  {"GetDeviceNewVersion", 0},
  {"GetUsageRecord", 0},
  {"GetSystemInfo", 0},
  {"GetNetworkInfo", 0},
};

/***begin**zlz added for session's heartbeat  2011-11-08**/
#define STR_URL_HEARTBEAT_ASP "/heartBeat.asp"
#define LENGTH_OF_URL_HEARTBEAT_ASP 14
/***end**zlz added for session's heartbeat  2011-11-08**/
#define STR_BACLUP_CONFIGURE_FOLDER  "/cfgbak"
#define STR_HELP_DOWNLOADDIR_NAME "/help"
#define STR_SDSHARE_DOWNLOADDIR_NAME "/sdcardDownload"

#ifdef JRD_DEBUG_CATCH_LOG
#define STR_LOGS_DOWNLOADDIR_NAME "/logs/"
#endif

/**add by houailing:2012-09-06******/
extern void set_timer_clean();
/**add by houailing:2012-09-06******/

void websInitSessionInfo()
{
    int nPageCount = 0, i = 0;
    memset(g_sessionids, 0, sizeof(g_sessionids));
     nPageCount = sizeof(g_freeAccessDirs)/sizeof(g_freeAccessDirs[0]);
     for(i = 0; i < nPageCount; i++)
     {
         g_freeAccessDirs[i].nlen =  gstrlen(g_freeAccessDirs[i].accesslimitedname) ;
         trace(4, "websInitSessionInfo-----nlen[%i]:%d \n", i, g_freeAccessDirs[i].nlen);
     }

     nPageCount = sizeof(g_freeAccessPages)/sizeof(g_freeAccessPages[0]);
     for(i = 0; i < nPageCount; i++)
     {
         g_freeAccessPages[i].nlen =  gstrlen(g_freeAccessPages[i].accesslimitedname) ;
         trace(4, "websInitSessionInfo--g_freeAccessPages---nlen[%i]:%d \n", i, g_freeAccessPages[i].nlen);
     }


}

void websSetCookie(webs_t wp, int bloginFailed)
{

    if(1 == bloginFailed) //login failed
    {
        sprintf(wp->cookiebuf,  "%s=%d; Path=/; ego", STR_COOKIE_KEY, 1);
    }
    else
    {
        if(wp->nSessionid < (MAX_SESSION_COUNT+2)) //clear flag if it had sessionid. 
        {
            g_sessionids[wp->nSessionid] = 0;
        }

        g_cSessionPC = g_cSessionPC % MAX_SESSION_COUNT + 1;
        if(1 == g_cSessionPC) //sessionid value is 1 means that login is failed,  sessionid value is 0 means that user is logouted
        {
            g_cSessionPC++;
        }
        
         g_sessionids[g_cSessionPC] = 1;
        sprintf(wp->cookiebuf,  "%s=%d; Path=/; ego", STR_COOKIE_KEY, g_cSessionPC);
    }
    
    websSetRequestFlags(wp, wp->flags | WEBS_COOKIE);

    {
        int i = 0, icount = 0;
        for(i = 2; i < (MAX_SESSION_COUNT + 2); i++)
        {
            if(g_sessionids[i] == 1)
            {
                icount ++;
                trace(4, "\n******session info: sessionid:%d, total:%d ***************************\n", i, icount);
            }
        }
    }
}





void websClearCookie(webs_t wp)
{
        if(wp->nSessionid < (MAX_SESSION_COUNT+2))
        {
            g_sessionids[wp->nSessionid] = 0;
        }

        
        sprintf(wp->cookiebuf,  "%s=%d; Path=/; ego", STR_COOKIE_KEY, 0);
        websSetRequestFlags(wp, wp->flags | WEBS_COOKIE);
   
}

int websGetSessionid(webs_t wp)
{

    return wp->nSessionid;
}


int websGetcookie(webs_t wp, char *name, char **ppValue, int *pLen)
{
    char *pcStr, *pcTmpStr1, *pcTmpStr2;
    unsigned short len, tmpLen1, tmpLen2;
    if ( !(wp->flags & WEBS_COOKIE))
        return 1;
    
    pcStr = wp->cookiebuf;
    len = strlen(pcStr);
    
        pcTmpStr1 = gstrchr(pcStr, ';');
        if(pcTmpStr1)
        {
            pcStr = pcTmpStr1+2;
        }
        
        /* get the name */
        pcTmpStr2 = gstrchr(pcStr, '=');
        if (!pcTmpStr2)
            return 1;
        
        tmpLen1 = pcTmpStr2 - pcStr;
        
        /* ignore the whitespace */
        
        /* get the name */
        if (gstrncmp(name, pcStr, tmpLen1))
            return 1;
        
        /* get the value */
        pcTmpStr1 = pcTmpStr2 + 1;
        tmpLen2 = gstrlen(pcTmpStr1);
        
        /* ignore the whitespace */
        
        if (!tmpLen2)
            return 1;
        
        *ppValue = pcTmpStr1;
        *pLen = tmpLen2;
        
        return 0;

}

/***end**zlz added for maintean user state 2011-08-18**/

/***begin**zlz added for session managerment by ip + ie type 2011-10-15**/
void EGOSM_Init()
{
    g_sessionsTable = symOpen(MAX_COUNT_OF_SESSION);
    return;
}

void EGOSM_End()
{
    if (g_sessionsTable == INVALID_SYM_FD)
      return;
    symClose(g_sessionsTable);
    g_sessionsTable = INVALID_SYM_FD;
    return;
}

static int __SM_getBrowserType(webs_t wp, exploreTypeE * peType)
{
	if (NULL == wp->userAgent){
        *peType = eTYPE_UNKNOWN;
        return 0;
	}
  if(NULL != gstrstr(wp->userAgent,STR_USERAGENT_OF_IE))
  {
      *peType = eTYPE_IE;
  }
  else if(NULL != gstrstr(wp->userAgent,STR_USERAGENT_OF_EDGE))
  {
      *peType = eTYPE_CHROME;
  }
  else if(NULL != gstrstr(wp->userAgent,STR_USERAGENT_OF_FIREFOX))
  {
      *peType = eTYPE_FIREFOX;
  }
  else if(NULL != gstrstr(wp->userAgent,STR_USERAGENT_OF_OPERA)||gstrstr(wp->userAgent,STR_USERAGENT_OF_OPERA2))
  {
      *peType = eTYPE_OPERA;
  }
  else if(NULL != gstrstr(wp->userAgent,STR_USERAGENT_OF_CHROME))
  {
      *peType = eTYPE_CHROME;
  }
  else if(NULL != gstrstr(wp->userAgent,STR_USERAGENT_OF_SAFARI))
  {
      *peType = eTYPE_SAFARI;
  }
  else
  {
      *peType = eTYPE_UNKNOWN;
  }
  return 0;
}

static char * __SM_getSessionKey(webs_t wp, exploreTypeE * peType)
{
  static char strSessionKey[35]; //sizeof(wp->ipaddr)+2
  memset(strSessionKey, 0, sizeof(strSessionKey));

  if (SESSION_KEY_FORMAT == SESSION_FORMAT_IP)
  {
    gsprintf(strSessionKey, "%s_0", wp->ipaddr);
    *peType = eTYPE_UNKNOWN;
    return strSessionKey;
  }

  __SM_getBrowserType(wp, peType);
  gsprintf(strSessionKey, "%s_%d", wp->ipaddr, *peType);
    
  return strSessionKey;

}

    /*****************************************************************************

    ******************************************************************************/
void EGOSM_setSessionInfo(webs_t wp, e_login_status_type bloginFailed)
{
	exploreTypeE eType = eTYPE_UNKNOWN;
  char * pstrKey = __SM_getSessionKey(wp, &eType);
  int cnt = EGOSM_getLoginFailCnt(wp); // Connie add, 2014/9/5
  if (g_sessionsTable == INVALID_SYM_FD)
  {
    return;
  }
//MSG_SPRINTF_3(MSG_SSID_DFLT, MSG_LEGACY_HIGH,"EGOSM_setSessionInfo(), session: %s, browser: %d, status: %d .",pstrKey,eType,bloginFailed);
  if(E_LOGIN_FAIL == bloginFailed) //login failed
  {
      symEnter(g_sessionsTable, pstrKey, valueInteger((int) SESSIONID_FOR_LOGIN_FAILED), cnt); // Connie modify, 2014/9/5
  }
	else if( E_LOGIN_OTHER_HAS_LOGIN == bloginFailed)//someone has logined
	{
		symEnter(g_sessionsTable, pstrKey, valueInteger((int) SESSIONID_FOR_HAS_LOGIN), cnt); // Connie modify, 2014/9/5
	}
  else if(E_LOGIN_SUCCESS == bloginFailed)
  {
      symEnter(g_sessionsTable, pstrKey, valueInteger((int) SESSIONID_FOR_LOGIN_SUCCESS), cnt); // Connie modify, 2014/9/5
  }
  //add by PiFangsi 2015-06-23 [add force login] start
  else if(E_LOGIN_FORCE_LOGIN_SUCCESS == bloginFailed)
  {
		int i = 0;
		sym_t *sp;
		g_delcounts = 0;
		sp = symFirst(g_sessionsTable);
		while(sp && (g_delcounts < MAX_COUNT_FOR_DELETE_ITEM))
		{
			a_assert(sp->content.type == integer);
			int nsessionid = (sp->content.value.integer & 0xFF);

			if(nsessionid == SESSIONID_FOR_LOGIN_SUCCESS)
			{
				strncpy(g_delsessionids[g_delcounts], sp->name.value.string, MAX_LENGTH_OF_ITEMNAME-1);
				g_delcounts++;
			}

			sp = symNext(g_sessionsTable);
		}

		//delete login session node
		for(i = 0; i < g_delcounts; i++)
		{
			symDelete(g_sessionsTable, g_delsessionids[i]);
//			symEnter(g_sessionsTable, g_delsessionids[i], valueInteger((int) SESSIONID_FOR_FORCE_LOGOUT), cnt); 
//			set_timer_clean();
		}
		symEnter(g_sessionsTable, pstrKey, valueInteger((int) SESSIONID_FOR_LOGIN_SUCCESS), cnt); 
	}
	//add by PiFangsi 2015-06-23 [add force login] end
    
}

int EGOSM_clearSessionInfo(webs_t wp)
{
    exploreTypeE eType = eTYPE_UNKNOWN;
    char * pstrKey = __SM_getSessionKey(wp, &eType);
    if (g_sessionsTable == INVALID_SYM_FD)
    {
      return -1;
    }

    //MSG_SPRINTF_2(MSG_SSID_DFLT, MSG_LEGACY_HIGH,"EGOSM_clearSessionInfo(), session_str: %s, browser: %d .",pstrKey,eType);
    return symDelete(g_sessionsTable, pstrKey);
}

int EGOSM_getSessionID(webs_t wp)
{
    exploreTypeE eType = eTYPE_UNKNOWN;
    char *  pstrSessKey = __SM_getSessionKey(wp, &eType);
    sym_t *sp;
    a_assert(pstrSessKey && *pstrSessKey);

    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "session_str: %s\n",pstrSessKey);
    if (g_sessionsTable == INVALID_SYM_FD)
    {
      return SESSIONID_FOR_LOGIN_SUCCESS;
    }

    if ((sp = symLookup(g_sessionsTable, pstrSessKey)) != NULL) 
    {
        a_assert(sp->content.type == integer);
		//MSG_SPRINTF_2(MSG_SSID_DFLT, MSG_LEGACY_HIGH,"EGOSM_getSessionID(), session_str: %s, value: %d .",
		//				pstrSessKey,(sp->content.value.integer & 0xFF));
        if (SESSIONID_FOR_LOGIN_FAILED == (sp->content.value.integer & 0xFF)) 
        {
            return SESSIONID_FOR_LOGIN_FAILED;
        }

        if (SESSIONID_FOR_HAS_LOGIN == (sp->content.value.integer & 0xFF)) 
        {
            return SESSIONID_FOR_HAS_LOGIN;
        }
		
        return SESSIONID_FOR_LOGIN_SUCCESS;
    }

    return SESSIONID_FOR_LOGOUT;
}

int EGOSM_isUserLogined(webs_t wp)
{
    exploreTypeE eType = eTYPE_UNKNOWN;
    char *  pstrSessKey = __SM_getSessionKey(wp, &eType);
    sym_t *sp;
    a_assert(pstrSessKey && *pstrSessKey);
    if (g_sessionsTable == INVALID_SYM_FD)
    {
      return 1;
    }

    if ((sp = symLookup(g_sessionsTable, pstrSessKey)) != NULL) 
    {
        a_assert(sp->content.type == integer);
        if (SESSIONID_FOR_LOGIN_FAILED == (sp->content.value.integer & 0xFF)) 
        {
            symDelete(g_sessionsTable, pstrSessKey);
            return 0;
        }
        if (SESSIONID_FOR_HAS_LOGIN == (sp->content.value.integer & 0xFF)) 
        {
            symDelete(g_sessionsTable, pstrSessKey);
            return 0;
        }
      //  sp->content.value.integer = SESSIONID_FOR_LOGIN_SUCCESS;//by jz2339, patch third party's BUG
        return 1;  
    }

    return 0;  
}

/***end**zlz added for session managerment by ip + ie type 2011-10-15**/


int  EGOSM_getPeerInfo(webs_t wp, char * pstrIp, int * pnExploreType)
{
    if(0 == pstrIp || 0 == pnExploreType || 0 == wp)
    {    
        return -1;
    }

    //Modified By Tian Yiqing, The web api from App don't have user agent, but we need ipaddrs here. Start
    gstrcpy(pstrIp, wp->ipaddr);
    //Modified By Tian Yiqing, The web api from App don't have user agent, but we need ipaddrs here. End
    if (SESSION_KEY_FORMAT == SESSION_FORMAT_IP)
    {
      *pnExploreType = eTYPE_UNKNOWN;
      return 0;
    }
    __SM_getBrowserType(wp, pnExploreType);

    return 0;    
}


int  EGOSM_getFirstSessionInfo( char * pstrIp, int * pnSessionID)  // Connie modify, 2014/9/5
{
    sym_t *sp;
   if(0 == pstrIp || 0 == pnSessionID) // Connie modify, 2014/9/5
    {    
        return 0;
    }
    
    if (g_sessionsTable == INVALID_SYM_FD)
    {
      return 0;
    }
    sp = symFirst(g_sessionsTable);
    if(0 == sp)
    {
        return 0;    
    }
    
    a_assert(sp->content.type == integer);
    *pnSessionID = sp->content.value.integer & 0xFF ;//by jz2339, patch third party's BUG
    
    a_assert(sp->name.type == string);
    if(0 != sp->name.value.string)
    {
        gstrncpy(pstrIp, sp->name.value.string, gstrlen(sp->name.value.string));
    }
    //*pnExploreType = sp->arg; // Connie remove, 2014/9/5
    return 1;
    
    
}

int  EGOSM_getNextSessionInfo( char * pstrIp, int * pnSessionID) // Connie modify, 2014/9/5
{
    sym_t *sp;
   if(0 == pstrIp || 0 == pnSessionID) // Connie modify, 2014/9/5
    {    
        return 0;
    }
    
    if (g_sessionsTable == INVALID_SYM_FD)
    {
      return 0;
    }
    
    sp = symNext(g_sessionsTable);
    if(0 == sp)
    {
        return 0;    
    }
    
    a_assert(sp->content.type == integer);
    *pnSessionID = sp->content.value.integer & 0xFF ; //by jz2339, patch third party's BUG
    
    a_assert(sp->name.type == string);
    if(0 != sp->name.value.string)
    {
        gstrncpy(pstrIp, sp->name.value.string, gstrlen(sp->name.value.string));
    }
    return 1;
}

/***begin**zlz added for session's heartbeat  2011-11-08**/
int  EGOSM_processSessionTimeOut(void *params)
{
	int i = 0;
	int nTimeout = 0, sessid = SESSIONID_FOR_LOGIN_SUCCESS;
       sym_t *sp;
	g_delcounts = 0;
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "EGOSM_processSessionTimeOut():SESSION_TIMEOUT_COUNT:%d.\n",SESSION_TIMEOUT_COUNT);
  if (g_sessionsTable == INVALID_SYM_FD)
  {
    set_timer_clean();
    return 0;
  }
	
	//MSG_HIGH("EGOSM_processSessionTimeOut()",0,0,0);
       sp = symFirst(g_sessionsTable);
	while(sp && (g_delcounts < MAX_COUNT_FOR_DELETE_ITEM))
	{
		nTimeout = (sp->content.value.integer >> 8) & 0xFF;
		sessid = sp->content.value.integer & 0xFF;

		nTimeout++;
		sp->content.value.integer = ((nTimeout << 8) &0xff00) + sessid;
		//MSG_SPRINTF_2(MSG_SSID_DFLT, MSG_LEGACY_HIGH,"EGOSM_processSessionTimeOut(), session: %s  timeout: %d .",sp->name.value.string, nTimeout);

		if(nTimeout >= SESSION_TIMEOUT_COUNT)
		{
			strncpy(g_delsessionids[g_delcounts], sp->name.value.string, MAX_LENGTH_OF_ITEMNAME-1);
			g_delcounts++;
		}

		sp = symNext(g_sessionsTable);
	}

	//delete timeout's session node
	for(i = 0; i < g_delcounts; i++)
	{
		symDelete(g_sessionsTable, g_delsessionids[i]);
		set_timer_clean();
		//MSG_SPRINTF_1(MSG_SSID_DFLT, MSG_LEGACY_HIGH,"EGOSM_processSessionTimeOut(), clear session: %s .",g_delsessionids[i]);
	}
  return 0;
}

// Connie add start for limit login fail no more then 5 times, 2014/9/4
int  EGOSM_getLoginFailCnt(webs_t wp)
{
    int LoginFail_cnt = 0;
    exploreTypeE eType = eTYPE_UNKNOWN;
    char *  pstrSessKey = __SM_getSessionKey(wp, &eType);
    sym_t *sp;
    a_assert(pstrSessKey && *pstrSessKey);

    if (g_sessionsTable == INVALID_SYM_FD)
    {
      return LoginFail_cnt;
    }

    if ((sp = symLookup(g_sessionsTable, pstrSessKey)) != NULL) 
    {
      LoginFail_cnt = sp->arg;
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "sp->arg: %d\n", sp->arg);
    }
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "sp: %p\n", sp);
    return LoginFail_cnt;
}

int  EGOSM_setLoginFailCnt(webs_t wp, int cnt)
{
	 exploreTypeE eType = eTYPE_UNKNOWN;
   char * pstrKey = __SM_getSessionKey(wp, &eType);
	 
   if (g_sessionsTable == INVALID_SYM_FD)
   {
      return -1;
   }
   symEnter(g_sessionsTable, pstrKey, valueInteger(EGOSM_getSessionID(wp)), cnt);  
   return 0;
}
// Connie add end for limit login fail no more then 5 times, 2014/9/4

int EGOSM_resetSessionHeartbeat(webs_t wp)
{
    exploreTypeE eType = eTYPE_UNKNOWN;
    char *  pstrSessKey = __SM_getSessionKey(wp, &eType);
    sym_t *sp;
    a_assert(pstrSessKey && *pstrSessKey);

	//MSG_HIGH("EGOSM_resetSessionHeartbeat()",0,0,0);
	 if (g_sessionsTable == INVALID_SYM_FD)
   {
      return -1;
   }
    if ((sp = symLookup(g_sessionsTable, pstrSessKey)) != NULL) 
    {
        a_assert(sp->content.type == integer);
        sp->content.value.integer = SESSIONID_FOR_LOGIN_SUCCESS;
        //MSG_SPRINTF_1(MSG_SSID_DFLT, MSG_LEGACY_HIGH,"EGOSM_resetSessionHeartbeat(), reset session: %s timeout.",pstrSessKey);
        return 1; 
    }
	
    return 0;
}
/***end**zlz added for session's heartbeat  2011-11-08**/

/*********************************** Code *************************************/
/*
 *	Determine if this request should be honored
 */

static char *jrd_certification_key = "KSDHSDFOGQ5WERYTUIQWERTYUISDFG1HJZXCVCXBN2GDSMNDHKVKFsVBNf";

int websSecurityHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
						char_t *url, char_t *path, char_t *query)
{
  char_t			*type, *userid, *password, *accessLimit;
  int				flags, nRet;
  accessMeth_t	am;
  /***begin**zlz added for maintean user state 2011-08-18**/
  char_t * pValue = 0;
  int nValueLen = 0;
  int nPageCount = 0, i = 0;
  /***end**zlz added for maintean user state 2011-08-18**/

  a_assert(websValid(wp));
  a_assert(url && *url);
  a_assert(path && *path);
  /*
  *	Get the critical request details
  */
  type = websGetRequestType(wp);
  password = websGetRequestPassword(wp);
  userid = websGetRequestUserName(wp);
  flags = websGetRequestFlags(wp);
  /*
  *	Get the access limit for the URL.  Exit if none found.
  */
  /******************************add by houailing:2012-11-07****************************************************/
#ifdef JRD_DEBUG_CATCH_LOG
  if( (NULL != gstrstr(url,STR_BACLUP_CONFIGURE_FOLDER)) || (NULL != gstrstr(url,STR_HELP_DOWNLOADDIR_NAME))
  ||(NULL != gstrstr(url,STR_LOGS_DOWNLOADDIR_NAME)) || (NULL != gstrstr(url,STR_SDSHARE_DOWNLOADDIR_NAME)))
  {
    wp->isDownloadfile = 1;
  }
#else    
  if( (NULL != gstrstr(url,STR_BACLUP_CONFIGURE_FOLDER)) || (NULL != gstrstr(url,STR_HELP_DOWNLOADDIR_NAME))
  ||(NULL != gstrstr(url,STR_SDSHARE_DOWNLOADDIR_NAME)))
  {
    wp->isDownloadfile = 1;
  }
#endif
  /************************end******************************************/
  if(url && 0 == gstrncmp(T("/jrd/webapi"), path, gstrlen(T("/jrd/webapi"))))
  {
    json_object * object_web_data = NULL;
    json_object * object_method = NULL;
    char_t  *method;

    if(!wp->referer 
    || ((gstrncmp(wp->referer + gstrlen(T("http://")), websIpaddr, gstrlen(websIpaddr))
        || *(wp->referer + gstrlen(T("http://")) + gstrlen(websIpaddr)) != '/')
      &&(gstrncmp(wp->referer + gstrlen(T("http://")), websHost, gstrlen(websHost))
        || *(wp->referer + gstrlen(T("http://")) + gstrlen(websHost)) != '/')))
    {
      char id[MAX_ID_LENGTH] = {0};
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "referer error! web IP:%s, web host:%s, referer: %s\n",websIpaddr, websHost, wp->referer);
      json_webs_get_id(wp, id, MAX_ID_LENGTH);
      json_webs_rsp_err(wp,WEBS_REFERER_ERROR,id);
      return 1;
    }
    
    if(!g_webs_data.json_method_login_check && !g_webs_data.json_method_token_check)
      return 0;

    object_web_data = json_tokener_parse(wp->postData);
    
    if( is_error(object_web_data) ) 
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "parser data fail!\n");
      return 0;
    }

    object_method = json_object_object_get(object_web_data, "method");
    if(object_method == NULL) 
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Can't get object_method\n");
      json_object_put(object_web_data);
      return 0;
    }

    method = json_object_get_string(object_method);

    if(0 == gstrcmp(method, "ActiveSimlock")||0 == gstrcmp(method, "SetTelnetSwitch"))//for special requests
    {
      json_object_put(object_web_data);
      return 0;
    }
    
    if (g_webs_data.json_method_token_check && (!wp->cer_key || strcmp(jrd_certification_key,wp->cer_key)))
    {
      char id[MAX_ID_LENGTH] = {0};
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "key is incorrect of method:%s, key:%s\n",method, wp->cer_key);
      json_object_put(object_web_data);
      json_webs_get_id(wp, id, MAX_ID_LENGTH);
      json_webs_rsp_err(wp,WEBS_TOKEN_ERROR,id);
      return 1;
    }

    nPageCount = sizeof(g_freeAccessMethods)/sizeof(g_freeAccessMethods[0]);
    for(i = 0; i < nPageCount; i++)
    {
      if(0 == gstrcmp(method, g_freeAccessMethods[i].accesslimitedname))
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "free method:%s\n",method);
        wp->flags &= ~WEBS_COOKIE;
        json_object_put(object_web_data);
        return 0;
      }
    }
    if(g_webs_data.json_method_login_check)
    {
      if(E_WEBS_UI_PW_NOT_NEEDED == g_webs_data.webs_uipwstate || TRUE == EGOSM_isUserLogined(wp))
      {
        wp->flags &= ~WEBS_COOKIE;
        
        if(g_webs_data.json_method_token_check)
        {
          if(!wp->token || strcmp(jrd_webs_token_str_extern,wp->token)!=0)
          {
            char id[MAX_ID_LENGTH] = {0};
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "token is incorrect of method:%s, wp->token:%s\n",method,wp->token);
            
            json_object_put(object_web_data);
            json_webs_get_id(wp, id, MAX_ID_LENGTH);
            json_webs_rsp_err(wp,WEBS_TOKEN_ERROR,id);
            return 1;
          }
        }
      }
      else
      {
        char id[MAX_ID_LENGTH] = {0};
        
        json_object_put(object_web_data);
        json_webs_get_id(wp, id, MAX_ID_LENGTH);
        json_webs_rsp_err(wp,WEBS_NEED_LOGIN,id);
        return 1;
      }
    }
    
    json_object_put(object_web_data);
    return 0;
  }
  else if(url 
      && (0 == gstrncmp(T("/goform"), path, gstrlen(T("/goform")))
          ||0 == gstrncmp(T("/cfgbak"), path, gstrlen(T("/cfgbak"))))
         )
  {
    if(!wp->referer 
    || ((gstrncmp(wp->referer + gstrlen(T("http://")), websIpaddr, gstrlen(websIpaddr))
        || *(wp->referer + gstrlen(T("http://")) + gstrlen(websIpaddr)) != '/')
      &&(gstrncmp(wp->referer + gstrlen(T("http://")), websHost, gstrlen(websHost))
        || *(wp->referer + gstrlen(T("http://")) + gstrlen(websHost)) != '/')))
    {
      char id[MAX_ID_LENGTH] = {0};
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "referer error! web IP:%s, web host:%s, referer: %s\n",websIpaddr, websHost, wp->referer);
      json_webs_get_id(wp, id, MAX_ID_LENGTH);
      json_webs_rsp_err(wp,WEBS_REFERER_ERROR,id);
      return 1;
    }
  }

  /***begin**zlz added for maintean user state 2011-08-18**/
  nPageCount = sizeof(g_freeAccessDirs)/sizeof(g_freeAccessDirs[0]);
  for(i = 0; i < nPageCount; i++)
  {
    if(url && 0 == memcmp(url, g_freeAccessDirs[i].accesslimitedname, g_freeAccessDirs[i].nlen))
    {
      trace(4, "****cookie****access to free free free****** page url:%s  \n", 0 == url?"":url);
      wp->flags &= ~WEBS_COOKIE;
      return 0;
    }
  }

  nPageCount = sizeof(g_freeAccessPages)/sizeof(g_freeAccessPages[0]);
  for(i = 0; i < nPageCount; i++)
  {
    //if httprequest access to  free page, passed 
    if(url && 0 == memcmp(url, g_freeAccessPages[i].accesslimitedname, g_freeAccessPages[i].nlen))
    {
      trace(4, "****cookie****22222 access to free page , url:%s  \n", 0 == url?"":url);
      wp->flags &= ~WEBS_COOKIE;
      return 0;
    }
  }
  /***begin**zlz added for session managerment by ip + ie type 2011-10-15**/
  if(1 == EGOSM_isUserLogined(wp))
  {
    // trace(4, "****EGOSM_isUserLogined == 1, url:%s  \n", 0 == url?"":url);
    wp->flags &= ~WEBS_COOKIE;

    /***begin**zlz added for session's heartbeat  2011-11-08**/
    //webserver reset session's timeout if explorer access /heartBeat.asp 
    if(url && 0 == memcmp(url, STR_URL_HEARTBEAT_ASP, LENGTH_OF_URL_HEARTBEAT_ASP))
    {
      EGOSM_resetSessionHeartbeat(wp);
    }
    /***end**zlz added for session's heartbeat  2011-11-08**/

    return 0;
  }
  /***end  **zlz added for session managerment by ip + ie type 2011-10-15**/

  trace(4, "access to limited url:%s, path:%s,seesionid:%d \n", 0 == url?"":url, 0==path?"":path,wp->nSessionid);
  wp->flags &= ~WEBS_COOKIE;
  wp->nSessionid = 0;
  websClearCookie(wp);
  //websResponse(wp, 200, "<script>top.location.href='../index.asp';</script>\r\n", 0);
  /*********modify by houailing 2013-03-09***********/
  if (wp->b_default_unredirect == FALSE) 	 
  {
    websResponse(wp, 200, "<script>top.location.href='../index.html';</script>\r\n", 0);
  }	
  return 1;
  /***end**zlz added for maintean user state 2011-08-18**/
  accessLimit = umGetAccessLimit(path);
  if (accessLimit == NULL) {
    return 0;
  }
   
  /*
  *	Check to see if URL must be encrypted
  */
#ifdef WEBS_SSL_SUPPORT
  nRet = umGetAccessLimitSecure(accessLimit);
  if (nRet && ((flags & WEBS_SECURE) == 0)) {
    websStats.access++;
    websError(wp, 405, T("Access Denied\nSecure access is required."));
    trace(3, T("SEC: Non-secure access attempted on <%s>\n"), path);
    /* bugfix 5/24/02 -- we were leaking the memory pointed to by
    * 'accessLimit'. Thanks to Simon Byholm.
    */
    bfree(B_L, accessLimit);
    return 1;
  }
#endif

  /*
  *	Get the access limit for the URL
  */
  am = umGetAccessMethodForURL(accessLimit);

  nRet = 0;
  if ((flags & WEBS_LOCAL_REQUEST) && (debugSecurity == 0)) {
  /*
  *		Local access is always allowed (defeat when debugging)
  */
  } else if (am == AM_NONE) {
    /*
    *		URL is supposed to be hidden!  Make like it wasn't found.
    */
    websStats.access++;
    websError(wp, 404, T("Page Not Found"));
    nRet = 1;
  } else 	if (userid && *userid) {
    if (!umUserExists(userid)) {
      websStats.access++;
      websError(wp, 401, T("Access Denied\nUnknown User"));
      trace(3, T("SEC: Unknown user <%s> attempted to access <%s>\n"), 
      userid, path);
      nRet = 1;
    } else if (!umUserCanAccessURL(userid, accessLimit)) {
      websStats.access++;
      websError(wp, 403, T("Access Denied\nProhibited User"));
      nRet = 1;
    } else if (password && * password) {
      char_t * userpass = umGetUserPassword(userid);
      if (userpass) {
        if (gstrcmp(password, userpass) != 0) {
          websStats.access++;
          websError(wp, 401, T("Access Denied\nWrong Password"));
          trace(3, T("SEC: Password fail for user <%s>")
          T("attempt to access <%s>\n"), userid, path);
          nRet = 1;
        } else {
        /*
        *					User and password check out.
        */
        }

        bfree (B_L, userpass);
      }
#ifdef DIGEST_ACCESS_SUPPORT
    } else if (flags & WEBS_AUTH_DIGEST) {

      char_t *digestCalc;

      /*
      *			Check digest for equivalence
      */
      wp->password = umGetUserPassword(userid);

      a_assert(wp->digest);
      a_assert(wp->nonce);
      a_assert(wp->password);

      digestCalc = websCalcDigest(wp);
      a_assert(digestCalc);

      if (gstrcmp(wp->digest, digestCalc) != 0) {
        bfree (B_L, digestCalc);
        digestCalc = websCalcUrlDigest(wp);
        a_assert(digestCalc);
        if (gstrcmp(wp->digest, digestCalc) != 0) {
          websStats.access++;

          websError(wp, 401, T("Access Denied\nWrong Password"));
          nRet = 1;
        }
      }

      bfree (B_L, digestCalc);
#endif
    } else {
      /*
      *			No password has been specified
      */
#ifdef DIGEST_ACCESS_SUPPORT
      if (am == AM_DIGEST) {
        wp->flags |= WEBS_AUTH_DIGEST;
      }
#endif
      websStats.errors++;
      websError(wp, 401, 
      T("Access to this document requires a password"));
      nRet = 1;
    }
  } else if (am != AM_FULL) {
    /*
    *		This will cause the browser to display a password / username
    *		dialog
    */
#ifdef DIGEST_ACCESS_SUPPORT
    if (am == AM_DIGEST) {
    wp->flags |= WEBS_AUTH_DIGEST;
    }
#endif
    websStats.errors++;
    websError(wp, 401, T("Access to this document requires a User ID"));
    nRet = 1;
  }
  bfree(B_L, accessLimit);

  return nRet;
}

/******************************************************************************/
/*
 *	Delete the default security handler
 */

void websSecurityDelete(void)
{
	websUrlHandlerDelete(websSecurityHandler);
}

/******************************************************************************/
/*
 *	Store the new password, expect a decoded password. Store in websPassword in 
 *	the decoded form.
 */

void websSetPassword(char_t *password)
{
	a_assert(password);

	gstrncpy(websPassword, password, TSZ(websPassword));
}

/******************************************************************************/
/*
 *	Get password, return the decoded form
 */

char_t *websGetPassword(void)
{
	return bstrdup(B_L, websPassword);
}

/******************************************************************************/


