// win_web_api_pro.cpp : Defines the entry point for the DLL application.
//

//#include "win_web_api_pro.h"
#include "webs.h"
#include	"wsIntrn.h"

//#include "msg.h"
#define WIN_WEB_API

typedef void *ego_webs_t;

#ifdef __cplusplus
extern "C"
{
#endif

    extern int ego_websvxmain();
    //extern char *__websGetLangString(char *pstrkey, int flag);

    typedef void(*LPFnFormCallback)(webs_t wp, char_t *path, char_t *query);
    typedef int(*LPFnAspCallback)(int ejid, webs_t wp, int argc, char_t **argv);

    /*响应请求页面*/
    WIN_WEB_API void ASPE_websReSponse(ego_webs_t pwebs, char *pwebname)
    {
        websResponse((webs_t)pwebs, 200, 0, pwebname);
        return ;
    }


    /*重定向页面*/
    WIN_WEB_API void ASPE_websRedirect(ego_webs_t pwebs, char *pwebname)
    {
        websRedirect((webs_t)pwebs, pwebname);
        return ;
    }

    //绑定一个 c 过程函数对应到一个用户输入表单（通常是通过“INPUT name=”标识）
    WIN_WEB_API int ASPE_websFormDefine(char_t *name, void(*fn)(ego_webs_t wp,
        char_t *path, char_t *query))
    {
        return websFormDefine(name, (LPFnFormCallback)fn);
    }



    //绑定一个 c 过程函数对应到一个 ASP 函数
    WIN_WEB_API int ASPE_websAspDefine(char_t *name, int(*fn)(int ejid,
        ego_webs_t wp, int argc, char_t **argv))
    {
        return websAspDefine(name, (LPFnAspCallback)fn);
    }


    //获取http 查询参数的key = value 值
    WIN_WEB_API char *ASPE_websGetVar(ego_webs_t wp, char *var, char
        *defaultGetValue)
    {
        return websGetVar((webs_t)wp, var, defaultGetValue);
    }


    // asp页面的asp脚本函数的输出内容
    WIN_WEB_API int ASPE_websWrite(ego_webs_t wp, char_t *fmt, ...)
    {
        va_list vargs;
        char_t *buf;
        int rc;

        a_assert(websValid(wp));

        va_start(vargs, fmt);

        buf = NULL;
        rc = 0;

        if (fmtValloc(&buf, WEBS_BUFSIZE, fmt, vargs) >= WEBS_BUFSIZE)
        {
            trace(0, T("webs: websWrite lost data, buffer overflow\n"));
        }

        va_end(vargs);
        a_assert(buf);
        if (buf)
        {
            rc = websWriteBlock((webs_t)wp, buf, gstrlen(buf));
            bfree(B_L, buf);
        }
        return rc;

    }

	/*
	** for output large string buffer to browser
	** by jz2339
	** 2011-9-21
	*/
	WIN_WEB_API int ASPE_websWriteLargeStr(ego_webs_t wp, char_t *buf)
    {
        if(!buf)
			return 0;

        return websWriteBlock((webs_t)wp, buf, gstrlen(buf));
    }
	
    //启动webserver
    #if 0
    WIN_WEB_API int ASPE_webserverStart()
    {
        return ego_websvxmain();
    }
    #endif

    //结束webserver
    WIN_WEB_API int ASPE_webserverEnd()
    {
        return 0;
    }

   /* WIN_WEB_API char *ASPE_websGetLangString(char *pstrkey, int flag)
    {
        return； //__websGetLangString(pstrkey, flag);
    }*/

    WIN_WEB_API int ASPE_websGetFuncParam(int argc, char_t **argv, char_t *fmt,
        ...)
    {
        va_list vargs;
        char_t *cp,  **sp;
        int *ip;
        int argn;

        va_start(vargs, fmt);

        if (argv == NULL)
        {
            return 0;
        }

        for (argn = 0, cp = fmt; cp &&  *cp && argv[argn];)
        {
            if (*cp++ != '%')
            {
                continue;
            }

            switch (*cp)
            {
                case 'd':
                    ip = va_arg(vargs, int*);
                    *ip = gatoi(argv[argn]);
                    break;

                case 's':
                    sp = va_arg(vargs, char_t **);
                    *sp = argv[argn];
                    break;

                default:
                    /*
                     *			Unsupported
                     */
                    a_assert(0);
            }
            argn++;
        }

        va_end(vargs);
        return argn;

    }
#if 0
    WIN_WEB_API int ASPE_setUploadfileDataCallback(ego_webs_t wp, /*int(*fn)
        (struct websRec *wp, unsigned char *pfileData, int nDatasize, void
        *userContext)*/upfileDataFnCallback fn, void *userContext)

    {
        /*((webs_t)wp)->pfn_upfileDataFnCallback = fn;
        ((webs_t)wp)->pupfileUsercontext = userContext;*/
        webs_t webp = (webs_t)wp;
		webp->pfn_upfileDataFnCallback = fn;
		webp->pupfileUsercontext = userContext;
			
        return 0;
    }
#endif
    /***begin**zlz added for session managerment by ip + ie type 2011-10-15**/
    extern void EGOSM_setSessionInfo(webs_t wp, e_login_status_type bloginFailed);
    extern int EGOSM_clearSessionInfo(webs_t wp);
    extern int EGOSM_getSessionID(webs_t wp);
    /***end **zlz added for session managerment by ip + ie type 2011-10-15**/

extern int  EGOSM_getPeerInfo(webs_t wp, char * pstrIp, int * pnExploreType);
extern int  EGOSM_getFirstSessionInfo( char * pstrIp, int * pnSessionID); // Connie modify, 2014/9/5
extern int  EGOSM_getNextSessionInfo( char * pstrIp, int * pnSessionID); // Connie modify, 2014/9/5

    /***begin**zlz added for maintean user state 2011-08-18**/
    // Connie add start for limit login fail no more then 5 times, 2014/9/4
    extern int  EGOSM_getLoginFailCnt(webs_t wp);        
    extern int  EGOSM_setLoginFailCnt(webs_t wp, int cnt);        
    // Connie add end for limit login fail no more then 5 times, 2014/9/4

    extern void websSetCookie(webs_t wp,  int bloginFailed);
    extern void websClearCookie(webs_t wp);
    extern int websGetSessionid(webs_t wp);


    /*****************************************************************************
    功能：通知webserver 用户登录结果
    输入：wp - web server  句柄 bloginFailed--0:登录成功 1:登录失败, 2: 已经有人登录
    输出：
    返回：0：成功 -1：失败
    说明： 
    ******************************************************************************/
    WIN_WEB_API int ASPE_userLogin(ego_webs_t wp, e_login_status_type bloginFailed)
    {
        //   websSetCookie((webs_t)wp, bloginFailed);

        /***begin**zlz added for session managerment by ip + ie type 2011-10-15**/
        EGOSM_setSessionInfo((webs_t)wp, bloginFailed);
        /***end **zlz added for session managerment by ip + ie type 2011-10-15**/
        return 0;
    }


    /*****************************************************************************
    功能：通知webserver 用户注销成功
    输入：
    输出：
    返回：0：成功 -1：失败
    说明： 
    ******************************************************************************/
    WIN_WEB_API int  ASPE_userLogout(ego_webs_t wp)
     {
        //websClearCookie(((webs_t)wp));
        /***begin**zlz added for session managerment by ip + ie type 2011-10-15**/
        return EGOSM_clearSessionInfo(((webs_t)wp));
        /***end **zlz added for session managerment by ip + ie type 2011-10-15**/
    }

    /*****************************************************************************
    功能：获取会话id
    输入：
    输出：
    返回：0：用户未登录 1:用户登录失败   >1 :当前会话id
    说明： 
    ******************************************************************************/
    WIN_WEB_API int  ASPE_getSessionID(ego_webs_t wp)
     {
        //return websGetSessionid(((webs_t)wp));
        /***begin**zlz added for session managerment by ip + ie type 2011-10-15**/
        int sessionID = EGOSM_getSessionID(((webs_t)wp));
        //printf("\n--------ASPE_getSessionID : %d \n", sessionID);
        return sessionID;
        /***end **zlz added for session managerment by ip + ie type 2011-10-15**/
    }

    /***end**zlz added for maintean user state 2011-08-18**/

	
     /***begin**zlz added for session managerment by ip + ie type 2011-10-15**/
    /*****************************************************************************
    功能：获取当前http client的ip 和浏览器类型
    输入：wp - web server  句柄 
    输出：pstrIp ----- http client的ip地址
                    pnExploreType -----浏览器类型   0: 未知类型 1:ie   2:chrome    3:firefox
    返回：0：成功 -1：失败
    说明： 
    ******************************************************************************/
    WIN_WEB_API int   ASPE_getPeerInfo(ego_webs_t  wp, char * pstrIp, int * pnExploreType)
    {
        return  EGOSM_getPeerInfo( wp, pstrIp, pnExploreType);
    }

    /*****************************************************************************
    功能：获取webserver 会话表的第一个会话信息
    输入： 
    输出：pstrIp ----- http client的ip地址
                    pnExploreType -----浏览器类型   0: 未知类型 1:ie   2:chrome    3:firefox
                    pnSessionID -------会话id              0:未登陆  1:登陆失败(用户名或密码错)    5:有效会话id
    返回：1：成功 0：失败
    说明：返回值为0，说明会话表为空；
    ******************************************************************************/
    WIN_WEB_API int   ASPE_getFirstSessionInfo( char * pstrIp, int * pnSessionID) // Connie modify, 2014/9/5
    {
        return  EGOSM_getFirstSessionInfo(pstrIp, pnSessionID); // Connie modify, 2014/9/5
    }


    /*****************************************************************************
    功能：获取webserver 会话表的下一个会话信息
    输入： 
    输出：pstrIp ----- http client的ip地址
                    pnExploreType -----浏览器类型   0: 未知类型 1:ie   2:chrome    3:firefox
                    pnSessionID -------会话id              0:未登陆  1:登陆失败(用户名或密码错)    5:有效会话id
    返回：1：成功 0：失败
    说明：ASPE_getFirstSessionInfo 调用一次、ASPE_getNextSessionInfo 调用多次，遍历整个会话表
    ******************************************************************************/
    WIN_WEB_API int   ASPE_getNextSessionInfo( char * pstrIp, int * pnSessionID) // Connie modify, 2014/9/5
    {
        return  EGOSM_getNextSessionInfo(pstrIp, pnSessionID);         // Connie modify, 2014/9/5
    }
    /***end **zlz added for session managerment by ip + ie type 2011-10-15**/

    // Connie add start for limit login fail no more then 5 times, 2014/9/4
    WIN_WEB_API int  ASPE_getLoginFailCnt(ego_webs_t wp)
    {
        return  EGOSM_getLoginFailCnt(wp);        
    }
    WIN_WEB_API int  ASPE_setLoginFailCnt(ego_webs_t wp, int cnt)
    {
        return  EGOSM_setLoginFailCnt(wp, cnt);        
    }
    // Connie add end for limit login fail no more then 5 times, 2014/9/4

#ifdef __cplusplus
};
#endif

