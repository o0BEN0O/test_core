#ifndef __WEBS_DEFINE_H__
#define __WEBS_DEFINE_H__ 1
#include "win_web_api_pro.h"
#include "webs.h"
#include "uemf.h"
#include "wsIntrn.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include "sock_clt.h" // Connie add, 2014/11/15
#include <common/jrd_common_def.h>

typedef int (*get_func_type)(int eid, webs_t wp, int argc, char_t **argv);

typedef void (*post_func_type)(webs_t wp, char_t *path, char_t *query);

#define MAX_ID_LENGTH           10  // Connie add, 2014/9/8

#define WEBS_TOKEN_ERROR -32699
#define WEBS_NEED_LOGIN -32698
#define WEBS_REFERER_ERROR -32697

#define JRD_SNPRINTF(str, size, format...)        snprintf(str, size, format)

// Connie add start, 2015/12/25
typedef enum
{
  E_WEBS_PARAM_INVALID    = -1,
  E_WEBS_PARAM_MIN        = 0x00,
  E_WEBS_PARAM_USER_NAME  = E_WEBS_PARAM_MIN,
  E_WEBS_PARAM_PASSWORD,
  E_WEBS_PARAM_LOCK_WEB_UI,
  E_WEBS_PARAM_MAX_RETRY_TIMES,
  E_WEBS_PARAM_UNLOCK_TIME,
  E_WEBS_PARAM_REMEMBER_USRPWD,
  E_WEBS_PARAM_USRPWD_CHANGED,
  E_WEBS_PARAM_FREE_ACCESS,
  E_WEBS_PARAM_WEBS_SECURITY,
  E_WEBS_PARAM_WEBS_SECURITY_NOLOGIN,
  E_WEBS_PARAM_WEBS_UNCONNECTED_REDIRECT,
  E_WEBS_PARAM_UIPWSTATE,
  E_WEBS_PARAM_LOGIN_RECEVER_TIME,
  E_WEBS_PARAM_JSON_LOGIN_CHECK,
  E_WEBS_PARAM_JSON_TOKEN_CHECK,
  E_WEBS_PARAM_PW_USERNAME_ENCRYPT,
  E_WEBS_PARAM_SUPPORT_CONN_REDERECT,
  E_WEBS_PARAM_CONN_REDERECT_PAGE,
  E_WEBS_PARAM_WIFI_PASSWORD_CHANGED,
  E_WEBS_PARAM_ORIGINAL_PASSWORD,
  /* BEGIN: Added by Alvin, 2018/4/11 */
  E_WEBS_PARAM_PASSWORD_TYPE,
  /* END:   Added by Alvin, 2018/4/11 */
  E_WEBS_PARAM_MAX,
}e_jrd_webs_param_id;

typedef enum
{
  E_WEBS_UI_PW_STATE_MIN = -1,
  E_WEBS_UI_PW_DEFAULT,
  E_WEBS_UI_PW_NOT_NEEDED,
  E_WEBS_UI_PW_NEEDED,
  E_WEBS_UI_PW_STATE_MAX
}e_jrd_webs_password_state;

typedef enum
{
  E_WEBS_LOGIN_STATE_MIN = -1,
  E_WEBS_LOGIN_STATE_LOGOUT,
  E_WEBS_LOGIN_STATE_LOGIN,
  E_WEBS_LOGIN_STATE_USED_OUT,
  E_WEBS_LOGIN_STATE_MAX
}e_jrd_webs_login_state;
  /////////////////////add begin by wei.huang.sz/////////////////
typedef enum
{
  E_WEBS_WIFI_UI_PW_STATE_MIN = -1,
  E_WEBS_WIFI_UI_PW_DEFAULT,
  E_WEBS_WIFI_UI_PW_CHANGE,
  E_WEBS_WIFI_UI_PW_STATE_MAX
}e_jrd_webs_wifi_password_state;

  /////////////////////add end by wei.huang.sz/////////////////

/* BEGIN: Added by Alvin, 2018/4/11 */
typedef enum
{
    E_WEBS_PASSWORD_TYPE_DEFAULT,
    E_WEBS_PASSWORD_TYPE_RANDOM_FROMMAC,
    E_WEBS_PASSWORD_TYPE_L8_IMEI,
    /* BEGIN: Added by yuan.jiang, 2019/5/7 */
    E_WEBS_PASSWORD_TYPE_RDM_4_5_WORDS,/*3*/
    /* END:   Added by yuan.jiang, 2019/5/7*/
    E_WEBS_PASSWORD_TYPE_MAX
}e_jrd_webs_password_type;
/* END:   Added by Alvin, 2018/4/11 */
typedef struct jrd_webs_pridata_struct {
	char username[32];
	char password[32];
	char original_password[32];
  int  lock_web_ui;
  int  max_retry_times;
  int  unlock_time;       //unit: s
  int  remember_usrpwd;
  int  usrpwd_changed;
  int  free_access;
  int  webs_security;
  int  webs_security_nologin;
  int  webs_unconnected_redirect;
  e_jrd_webs_password_state  webs_uipwstate;
  int  webs_recover_time;
  int  json_method_login_check;
  int  json_method_token_check;
  int  webs_pw_username_encrypt;
  int  support_connet_redirect;
  char connect_redirect_page[64];
  e_jrd_webs_wifi_password_state  wifi_password_changed;
  /* BEGIN: Added by Alvin, 2018/4/11 */
  e_jrd_webs_password_type password_type;
  /* END:   Added by Alvin, 2018/4/11 */
  /* BEGIN: Added by yuan.jiang, 2019/5/7 */
  char password_md5[32+1];
  /* END:   Added by yuan.jiang, 2019/5/7*/
} jrd_webs_pridata_t;
// Connie add end, 2015/12/25

typedef struct {
  char     *func_name;
  get_func_type  func_ptr;
} webs_get_req_type;

typedef struct {
  char     *func_name;
  post_func_type func_ptr;
} webs_post_req_type;

typedef enum{
	WEBS_SET_SUCCESS         = 0x00,
	WEBS_SET_FAILED        = 0x01,	     
	WEBS_CALL_API_FAILED    = 0x02,
	WEBS_GET_UIPARA_WRONG   = 0x03,
} webs_asp_result_type;

typedef enum{
	WEBS_LOGOUT         = 0x00,
	WEBS_LOGIN_FAILED   = 0x01,
	WEBS_LOGIN_OTHER    = 0x02,
	WEBS_LOGIN_MORE     = 0x03,
	WEBS_LOGIN_GUEST    = 0x04,
	WEBS_LOGIN_SUCESS   = 0x05,
  WEBS_LOGIN_LIMIT    = 0x06,
  WEBS_LOGIN_ALREADY_LOGIN   = 0x07,
} webs_login_state;

struct webs_config_param{
	char username[32];
	char password[32];
};


//add by PiFangsi 2015-06-18 [add force login api] start
void json_webs_force_login(webs_t wp, char_t *path, char_t *query);
//add by PiFangsi 2015-06-18 [add force login api] end
extern int  EGOSM_processSessionTimeOut(void *params);

void webs_asp_init(void);
void webstnHeader(webs_t wp);
#ifdef JRD_EE_CUST_FEATURE_DEVINFO
void websJsonHeader_ee_cust(webs_t wp);
#endif
void websJsonHeader(webs_t wp, uint32, e_jrd_cust_flag_type flag_type); // Connie modify, 2014/9/8

int webs_init_database(void);
int webs_update_database(e_jrd_webs_param_id param_id);

int webs_get_imei(void);


#endif
