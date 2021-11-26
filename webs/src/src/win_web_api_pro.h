
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the WIN_WEB_API_PRO_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// WIN_WEB_API_PRO_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifndef __WIN_WEB_API_PRO_H__
#define __WIN_WEB_API_PRO_H__ 1

#include <common/comdef.h>
#define WIN_WEB_API

typedef char char_t; //web_frameif.h:211 has definition


typedef void *ego_webs_t;

struct websRec;
typedef int(*upfileDataFnCallback)(struct websRec *wp, unsigned char *pfileData, int nDatasize, void *userContext);

#ifdef __cplusplus
extern "C"
{
#endif
    extern void trace(int level, char_t *printParams, ...);

    /*****************************************************************************
    功能：通过key 从 语言资源文件获取 value值,语言资源文件名----"resource/stringtable.en"
    输入：pstrkey –  key名字
    输出：
    返回：value字符串。未找到key的value，返回NULL
     ******************************************************************************/
   // WIN_WEB_API char *ASPE_websGetLangString(char *pstrkey, int flag);


    /*****************************************************************************
    功能：获取 ASP 函数的输入参数
    输入：argc –  输入参数个数
    argv –  输入参数首地址
    fmt  –  参数输出格式，由调用者制定数据输出格式,只支持%s和%d 两种数据格式。
    格式需与ASP 脚本中对应函数输入参数保持一致
    输出：ASP 脚本中对应函数输入参数将根据 fmt 格式定义打印到对应变量中
    返回：成功获取的参数个数
    说明：argc，argv在 ASP 函数对应的回调函数中带上来。
     ******************************************************************************/
    WIN_WEB_API int ASPE_websGetFuncParam(int argc, char_t **argv, char_t *fmt,
        ...);

    /*****************************************************************************
    功能：响应请求页面
    输入：pwebs - web server  句柄
    pwebname - 返回页面名， 如：connect.asp
    输出：
    返回：
     ******************************************************************************/
    WIN_WEB_API void ASPE_websReSponse(ego_webs_t pwebs, char *pwebname);



    /*****************************************************************************
    功能：重定向页面
    输入：pwebs - web server  句柄
    pwebname - 重定向页面名， 如：connect.asp
    输出：
    返回：
     ******************************************************************************/
    WIN_WEB_API void ASPE_websRedirect(ego_webs_t pwebs, char *pwebname);


    /*****************************************************************************
    功能：绑定一个 c 过程函数对应到一个用户输入表单（通常是通过“INPUT name=”标识）
    输入：name – ASP 文件名（通过 ASP 脚本中“action=“dir/xxx.asp”   method=“POST””指定）
    fn – c 函数（需要功能实现模块实现的回调函数）
    输出：无
    返回：成功返回 0
    说明：在 fn 函数中，用户调用 websWrite 实现数据交换
     ******************************************************************************/
    WIN_WEB_API int ASPE_websFormDefine(char_t *name, void(*fn)(ego_webs_t wp,
        char_t *path, char_t *query));



    /*****************************************************************************
    功能：绑定一个 c 过程函数对应到一个 ASP 函数
    输入：name – ASP 函数
    fn – c 函数
    输出：无
    返回：成功返回 0
    说明：在 fn 函数中，用户调用 websWrite 实现数据交换
     ******************************************************************************/
    WIN_WEB_API int ASPE_websAspDefine(char_t *name, int(*fn)(int ejid,
        ego_webs_t wp, int argc, char_t **argv));


    /*****************************************************************************
    功能：从 POST 方法中获取用户输入数据
    输入：wp - web server  句柄
    var -  要获取的变量名，通常在脚本中以<INPUT name=“”>指定
    defaultGetValue –  当用户没有填写此值时，系统的缺省值
    输出：动态 HTML页面中内嵌的 ASP 脚本将被以上变参格式的数据替代
    返回：成功打印到动态 HTML页面的字节数
     ******************************************************************************/
    WIN_WEB_API char *ASPE_websGetVar(ego_webs_t wp, char *var, char
        *defaultGetValue);


    /*****************************************************************************
    功能：将数据传递给页面，在 UI 页面呈现相关数据
    输入：wp - web server  句柄
    fmt -变参，由调用者制定数据输出格式
    输出：动态 HTML页面中内嵌的 ASP 脚本将被以上变参格式的数据替代
    返回：成功打印到动态 HTML页面的字节数
    说明：
     ******************************************************************************/
    WIN_WEB_API int ASPE_websWrite(ego_webs_t wp, char_t *fmt, ...);
	WIN_WEB_API int ASPE_websWriteLargeStr(ego_webs_t wp, char_t *buf);

    /*****************************************************************************
    功能：启动webserver
    输入：
    输出：
    返回：0：成功 -1：失败
    说明：
     ******************************************************************************/
    WIN_WEB_API int ASPE_webserverStart();

    /*****************************************************************************
    功能：结束webserver
    输入：
    输出：
    返回：0：成功 -1：失败
    说明：
     ******************************************************************************/
    WIN_WEB_API int ASPE_webserverEnd();



    /*****************************************************************************
    功能：设置上传文件回调函数fn， webserver 调用fn把文件数据传给应用程序
    输入：int (*fn)(ego_webs_t wp,  unsigned char * pfileData, int nDatasize, void * userContext)
    fn:应用程序回调函数，webserver收到数据时调用
    fn 返回值: 1: webserver 能够继续调用fn， 0: fn 已经无效
    fn-->wp: webserver为http连接创建的句柄
    fn-->pfileData: 文件数据buf指针
    fn-->nDatasize:文件数据大小
    fn-->userContext: 应用程序上下文，webserver回调fn原本传回
    wp:    webserver为http连接创建的句柄
    userContext:  应用程序上下文，webserver回调fn原本传回
    输出：
    返回：0：成功 -1：失败
    说明：
     ******************************************************************************/
     #if 0
    WIN_WEB_API int ASPE_setUploadfileDataCallback(ego_webs_t wp, /*int(*fn)
        (struct websRec *wp, unsigned char *pfileData, int nDatasize, void
        *userContext)*/upfileDataFnCallback fn, void *userContext);
#endif

    /***begin**zlz added for maintean user state 2011-08-18**/

    /*****************************************************************************
    功能：通知webserver 用户登录结果
    输入：wp - web server  句柄 bloginFailed--0:登录成功 1:登录失败
    输出：
    返回：0：成功 -1：失败
    说明： 
    ******************************************************************************/
    WIN_WEB_API int ASPE_userLogin(ego_webs_t wp, int bloginFailed);


    /*****************************************************************************
    功能：通知webserver 用户注销成功
    输入：wp - web server  句柄
    输出：
    返回：0：成功 -1：失败
    说明： 
    ******************************************************************************/
    WIN_WEB_API int  ASPE_userLogout(ego_webs_t wp);


    /*****************************************************************************
    功能：获取会话id
    输入：wp - web server  句柄
    输出：
    返回：0：用户未登录 1:用户登录失败   >1 :当前会话id
    说明： 
    ******************************************************************************/
    WIN_WEB_API int  ASPE_getSessionID(ego_webs_t wp);

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
    WIN_WEB_API int   ASPE_getPeerInfo(ego_webs_t  wp, char * pstrIp, int * pnExploreType);


    /*****************************************************************************
    功能：获取webserver 会话表的第一个会话信息
    输入： 
    输出：pstrIp ----- http client的ip地址
                    pnExploreType -----浏览器类型   0: 未知类型 1:ie   2:chrome    3:firefox
                    pnSessionID -------会话id              0:未登陆  1:登陆失败(用户名或密码错)    5:有效会话id
    返回：1：成功 0：失败
    说明：返回值为0，说明会话表为空；
    ******************************************************************************/
    WIN_WEB_API int   ASPE_getFirstSessionInfo( char * pstrIp, int * pnSessionID); // Connie modify, 2014/9/5

    /*****************************************************************************
    功能：获取webserver 会话表的下一个会话信息
    输入： 
    输出：pstrIp ----- http client的ip地址
                    pnExploreType -----浏览器类型   0: 未知类型 1:ie   2:chrome    3:firefox
                    pnSessionID -------会话id              0:未登陆  1:登陆失败(用户名或密码错)    5:有效会话id
    返回：1：成功 0：失败
    说明：ASPE_getFirstSessionInfo 调用一次、ASPE_getNextSessionInfo 调用多次，遍历整个会话表
    ******************************************************************************/
    WIN_WEB_API int   ASPE_getNextSessionInfo( char * pstrIp, int * pnSessionID); // Connie modify, 2014/9/5
    /***end **zlz added for session managerment by ip + ie type 2011-10-15**/
    // Connie add start for limit login fail no more then 5 times, 2014/9/4
    WIN_WEB_API int  ASPE_getLoginFailCnt(ego_webs_t wp);
    WIN_WEB_API int  ASPE_setLoginFailCnt(ego_webs_t wp, int cnt);
    // Connie add end for limit login fail no more then 5 times, 2014/9/4
#ifdef __cplusplus
};
#endif

#endif /**__WIN_WEB_API_PRO_H__ **/
