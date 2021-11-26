
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
    ���ܣ�ͨ��key �� ������Դ�ļ���ȡ valueֵ,������Դ�ļ���----"resource/stringtable.en"
    ���룺pstrkey �C  key����
    �����
    ���أ�value�ַ�����δ�ҵ�key��value������NULL
     ******************************************************************************/
   // WIN_WEB_API char *ASPE_websGetLangString(char *pstrkey, int flag);


    /*****************************************************************************
    ���ܣ���ȡ ASP �������������
    ���룺argc �C  �����������
    argv �C  ��������׵�ַ
    fmt  �C  ���������ʽ���ɵ������ƶ����������ʽ,ֻ֧��%s��%d �������ݸ�ʽ��
    ��ʽ����ASP �ű��ж�Ӧ���������������һ��
    �����ASP �ű��ж�Ӧ����������������� fmt ��ʽ�����ӡ����Ӧ������
    ���أ��ɹ���ȡ�Ĳ�������
    ˵����argc��argv�� ASP ������Ӧ�Ļص������д�������
     ******************************************************************************/
    WIN_WEB_API int ASPE_websGetFuncParam(int argc, char_t **argv, char_t *fmt,
        ...);

    /*****************************************************************************
    ���ܣ���Ӧ����ҳ��
    ���룺pwebs - web server  ���
    pwebname - ����ҳ������ �磺connect.asp
    �����
    ���أ�
     ******************************************************************************/
    WIN_WEB_API void ASPE_websReSponse(ego_webs_t pwebs, char *pwebname);



    /*****************************************************************************
    ���ܣ��ض���ҳ��
    ���룺pwebs - web server  ���
    pwebname - �ض���ҳ������ �磺connect.asp
    �����
    ���أ�
     ******************************************************************************/
    WIN_WEB_API void ASPE_websRedirect(ego_webs_t pwebs, char *pwebname);


    /*****************************************************************************
    ���ܣ���һ�� c ���̺�����Ӧ��һ���û����������ͨ����ͨ����INPUT name=����ʶ��
    ���룺name �C ASP �ļ�����ͨ�� ASP �ű��С�action=��dir/xxx.asp��   method=��POST����ָ����
    fn �C c ��������Ҫ����ʵ��ģ��ʵ�ֵĻص�������
    �������
    ���أ��ɹ����� 0
    ˵������ fn �����У��û����� websWrite ʵ�����ݽ���
     ******************************************************************************/
    WIN_WEB_API int ASPE_websFormDefine(char_t *name, void(*fn)(ego_webs_t wp,
        char_t *path, char_t *query));



    /*****************************************************************************
    ���ܣ���һ�� c ���̺�����Ӧ��һ�� ASP ����
    ���룺name �C ASP ����
    fn �C c ����
    �������
    ���أ��ɹ����� 0
    ˵������ fn �����У��û����� websWrite ʵ�����ݽ���
     ******************************************************************************/
    WIN_WEB_API int ASPE_websAspDefine(char_t *name, int(*fn)(int ejid,
        ego_webs_t wp, int argc, char_t **argv));


    /*****************************************************************************
    ���ܣ��� POST �����л�ȡ�û���������
    ���룺wp - web server  ���
    var -  Ҫ��ȡ�ı�������ͨ���ڽű�����<INPUT name=����>ָ��
    defaultGetValue �C  ���û�û����д��ֵʱ��ϵͳ��ȱʡֵ
    �������̬ HTMLҳ������Ƕ�� ASP �ű��������ϱ�θ�ʽ���������
    ���أ��ɹ���ӡ����̬ HTMLҳ����ֽ���
     ******************************************************************************/
    WIN_WEB_API char *ASPE_websGetVar(ego_webs_t wp, char *var, char
        *defaultGetValue);


    /*****************************************************************************
    ���ܣ������ݴ��ݸ�ҳ�棬�� UI ҳ������������
    ���룺wp - web server  ���
    fmt -��Σ��ɵ������ƶ����������ʽ
    �������̬ HTMLҳ������Ƕ�� ASP �ű��������ϱ�θ�ʽ���������
    ���أ��ɹ���ӡ����̬ HTMLҳ����ֽ���
    ˵����
     ******************************************************************************/
    WIN_WEB_API int ASPE_websWrite(ego_webs_t wp, char_t *fmt, ...);
	WIN_WEB_API int ASPE_websWriteLargeStr(ego_webs_t wp, char_t *buf);

    /*****************************************************************************
    ���ܣ�����webserver
    ���룺
    �����
    ���أ�0���ɹ� -1��ʧ��
    ˵����
     ******************************************************************************/
    WIN_WEB_API int ASPE_webserverStart();

    /*****************************************************************************
    ���ܣ�����webserver
    ���룺
    �����
    ���أ�0���ɹ� -1��ʧ��
    ˵����
     ******************************************************************************/
    WIN_WEB_API int ASPE_webserverEnd();



    /*****************************************************************************
    ���ܣ������ϴ��ļ��ص�����fn�� webserver ����fn���ļ����ݴ���Ӧ�ó���
    ���룺int (*fn)(ego_webs_t wp,  unsigned char * pfileData, int nDatasize, void * userContext)
    fn:Ӧ�ó���ص�������webserver�յ�����ʱ����
    fn ����ֵ: 1: webserver �ܹ���������fn�� 0: fn �Ѿ���Ч
    fn-->wp: webserverΪhttp���Ӵ����ľ��
    fn-->pfileData: �ļ�����bufָ��
    fn-->nDatasize:�ļ����ݴ�С
    fn-->userContext: Ӧ�ó��������ģ�webserver�ص�fnԭ������
    wp:    webserverΪhttp���Ӵ����ľ��
    userContext:  Ӧ�ó��������ģ�webserver�ص�fnԭ������
    �����
    ���أ�0���ɹ� -1��ʧ��
    ˵����
     ******************************************************************************/
     #if 0
    WIN_WEB_API int ASPE_setUploadfileDataCallback(ego_webs_t wp, /*int(*fn)
        (struct websRec *wp, unsigned char *pfileData, int nDatasize, void
        *userContext)*/upfileDataFnCallback fn, void *userContext);
#endif

    /***begin**zlz added for maintean user state 2011-08-18**/

    /*****************************************************************************
    ���ܣ�֪ͨwebserver �û���¼���
    ���룺wp - web server  ��� bloginFailed--0:��¼�ɹ� 1:��¼ʧ��
    �����
    ���أ�0���ɹ� -1��ʧ��
    ˵���� 
    ******************************************************************************/
    WIN_WEB_API int ASPE_userLogin(ego_webs_t wp, int bloginFailed);


    /*****************************************************************************
    ���ܣ�֪ͨwebserver �û�ע���ɹ�
    ���룺wp - web server  ���
    �����
    ���أ�0���ɹ� -1��ʧ��
    ˵���� 
    ******************************************************************************/
    WIN_WEB_API int  ASPE_userLogout(ego_webs_t wp);


    /*****************************************************************************
    ���ܣ���ȡ�Ựid
    ���룺wp - web server  ���
    �����
    ���أ�0���û�δ��¼ 1:�û���¼ʧ��   >1 :��ǰ�Ựid
    ˵���� 
    ******************************************************************************/
    WIN_WEB_API int  ASPE_getSessionID(ego_webs_t wp);

    /***end**zlz added for maintean user state 2011-08-18**/

     /***begin**zlz added for session managerment by ip + ie type 2011-10-15**/
    /*****************************************************************************
    ���ܣ���ȡ��ǰhttp client��ip �����������
    ���룺wp - web server  ��� 
    �����pstrIp ----- http client��ip��ַ
                    pnExploreType -----���������   0: δ֪���� 1:ie   2:chrome    3:firefox
    ���أ�0���ɹ� -1��ʧ��
    ˵���� 
    ******************************************************************************/
    WIN_WEB_API int   ASPE_getPeerInfo(ego_webs_t  wp, char * pstrIp, int * pnExploreType);


    /*****************************************************************************
    ���ܣ���ȡwebserver �Ự���ĵ�һ���Ự��Ϣ
    ���룺 
    �����pstrIp ----- http client��ip��ַ
                    pnExploreType -----���������   0: δ֪���� 1:ie   2:chrome    3:firefox
                    pnSessionID -------�Ựid              0:δ��½  1:��½ʧ��(�û����������)    5:��Ч�Ựid
    ���أ�1���ɹ� 0��ʧ��
    ˵��������ֵΪ0��˵���Ự��Ϊ�գ�
    ******************************************************************************/
    WIN_WEB_API int   ASPE_getFirstSessionInfo( char * pstrIp, int * pnSessionID); // Connie modify, 2014/9/5

    /*****************************************************************************
    ���ܣ���ȡwebserver �Ự������һ���Ự��Ϣ
    ���룺 
    �����pstrIp ----- http client��ip��ַ
                    pnExploreType -----���������   0: δ֪���� 1:ie   2:chrome    3:firefox
                    pnSessionID -------�Ựid              0:δ��½  1:��½ʧ��(�û����������)    5:��Ч�Ựid
    ���أ�1���ɹ� 0��ʧ��
    ˵����ASPE_getFirstSessionInfo ����һ�Ρ�ASPE_getNextSessionInfo ���ö�Σ����������Ự��
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