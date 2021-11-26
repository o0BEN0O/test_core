#include <stdlib.h>
#include <stdio.h>
#include "jrd_db_common.h"
#include "webs_define.h"
#include "win_web_api_pro.h"
#include "jrd_timer.h"
#include "sock_clt.h"
#include <json-c/json.h>
#include <pthread.h>
#include "jrd_md5.h"
#define COUNTOF(array) sizeof(array)/sizeof(array[0])

#define WEBS_SET_PSW_SUCCESS 0
#define WEBS_SET_PSW_FAILED 1
#define WEBS_SET_PSW_WRONG_PASSWD 2
#define WEBS_SET_PSW_ORIGINAL_PASSWD 3


#define SESSIONID_FOR_LOGOUT  0
#define SESSIONID_FOR_LOGIN_FAILED  1
#define SESSIONID_FOR_HAS_LOGIN 2
#define SESSIONID_FOR_LOGIN_SUCCESS  5

/* BEGIN: Added by yuan.jiang, 2019/8/23 PN: webs password genreate when 1st startup */
#define JRD_DEV_IMEI_FILE "/cache/.imei"
#define JRD_WEBS_PWD_FLAG "/jrd-resource/resource/.webs_pwd_success"
/* END:   Added by yuan.jiang, 2019/8/23*/

#define WLAN_FILE_LOGIN_IP "/jrd-resource/resource/wlan/login_ip"
#define ADMIN_ORANGE_RANDOM_PSWD_LEN 8

static  int g_login_fail_count = 0;

static  int jrd_webs_token = 0;
static char jrd_webs_token_str[32] = {35,45,34,56,75,34,54,0};
char * jrd_webs_token_str_extern = jrd_webs_token_str;

//static  int jrd_webs_login_token = 0;
//static  char jrd_webs_login_token_str[32] = {35,45,63,56,65,34,54,0};

//char* jrd_default_token = "74623918";

static char *jrd_key="e5dl12XYVggihggafXWf0f2YSf2Xngd1";//1~9,A~Z,a~o

static  int g_login_fail_time_count = 0; 
static  pthread_t g_login_time_thrd;
/********************************************************************/
static timer_info_t login_check_timer = {0};  //Connie modified, 2016/02/25

// Connie add start, 2015/12/25
jrd_webs_pridata_t g_webs_data = {0};
static jrd_param_key_info_t jrd_webs_config_data_table[]=
{
  {&g_webs_data.username,         "username",         E_PARAM_STR, sizeof(g_webs_data.username), E_WEBS_PARAM_USER_NAME},
  {&g_webs_data.password,         "password",         E_PARAM_STR, sizeof(g_webs_data.username), E_WEBS_PARAM_PASSWORD},
  {&g_webs_data.lock_web_ui,      "lock_web_ui",      E_PARAM_INT, sizeof(int),                  E_WEBS_PARAM_LOCK_WEB_UI},
  {&g_webs_data.max_retry_times,  "max_retry_times",  E_PARAM_INT, sizeof(int),                  E_WEBS_PARAM_MAX_RETRY_TIMES},
  {&g_webs_data.unlock_time,      "unlock_time",      E_PARAM_INT, sizeof(int),                  E_WEBS_PARAM_UNLOCK_TIME},
  {&g_webs_data.remember_usrpwd,  "remember_usrpwd",  E_PARAM_INT, sizeof(int),                  E_WEBS_PARAM_REMEMBER_USRPWD},
  {&g_webs_data.free_access,      "free_access",      E_PARAM_INT, sizeof(int),                  E_WEBS_PARAM_FREE_ACCESS},
  {&g_webs_data.webs_security,  "webs_security",  E_PARAM_INT, sizeof(int),                  E_WEBS_PARAM_WEBS_SECURITY},
  {&g_webs_data.webs_security_nologin,  "webs_security_nologin",  E_PARAM_INT, sizeof(int),                  E_WEBS_PARAM_WEBS_SECURITY_NOLOGIN},
  {&g_webs_data.webs_unconnected_redirect,  "webs_unconnected_redirect",  E_PARAM_INT, sizeof(int),   E_WEBS_PARAM_WEBS_UNCONNECTED_REDIRECT},
  {&g_webs_data.webs_uipwstate,  "webs_uiwstate",  E_PARAM_INT, sizeof(int),   E_WEBS_PARAM_UIPWSTATE},
  {&g_webs_data.webs_recover_time,  "webs_recover_time",  E_PARAM_INT, sizeof(int),   E_WEBS_PARAM_UIPWSTATE},
  {&g_webs_data.json_method_login_check,  "json_method_login_check",  E_PARAM_INT, sizeof(int),   E_WEBS_PARAM_JSON_LOGIN_CHECK},
  {&g_webs_data.json_method_token_check,  "json_method_token_check",  E_PARAM_INT, sizeof(int),   E_WEBS_PARAM_JSON_TOKEN_CHECK},
  {&g_webs_data.webs_pw_username_encrypt,  "webs_pw_username_encrypt",  E_PARAM_INT, sizeof(int),   E_WEBS_PARAM_PW_USERNAME_ENCRYPT},
  {&g_webs_data.support_connet_redirect,   "support_connet_redirect",   E_PARAM_INT, sizeof(int),   E_WEBS_PARAM_SUPPORT_CONN_REDERECT},
  {g_webs_data.connect_redirect_page,     "connect_redirect_page",     E_PARAM_STR, sizeof(g_webs_data.connect_redirect_page),   E_WEBS_PARAM_CONN_REDERECT_PAGE},
  {&g_webs_data.wifi_password_changed,   "wifi_password_changed",   E_PARAM_INT, sizeof(int),                  E_WEBS_PARAM_WIFI_PASSWORD_CHANGED},
  {&g_webs_data.original_password,   "original_password",   E_PARAM_STR,sizeof(g_webs_data.username),                  E_WEBS_PARAM_ORIGINAL_PASSWORD},
  /* BEGIN: Added by Alvin, 2018/4/11 */
  {&g_webs_data.password_type,  "webs_password_type",  E_PARAM_INT, sizeof(int),   E_WEBS_PARAM_PASSWORD_TYPE},
  /* END:   Added by Alvin, 2018/4/11 */
  {NULL,NULL,0,0,0},
};

static jrd_param_table_info_t jrd_webs_config_data_table_info = 
{
  "webs_config",
  jrd_webs_config_data_table,
};

static jrd_param_key_info_t jrd_webs_info_data_table[]=
{
  {&g_webs_data.usrpwd_changed,   "usrpwd_changed",   E_PARAM_INT, sizeof(int),                  E_WEBS_PARAM_USRPWD_CHANGED},
  {NULL,NULL,0,0,0},
};

static jrd_param_table_info_t jrd_webs_info_data_table_info = 
{
  "webs_info",
  jrd_webs_info_data_table,
};

// Connie add end, 2015/12/25


/****************************************************************/
//check post data 

#define PARAM_INT (0)
#define PARAM_STR (1)

#define INVALID_VALUE   (LONG_MAX)
#define MAX_PARAM_LENGTH        32  // Connie add, 2014/9/5

typedef struct {
  char         *name;
  const int     type;  /* 0: INT; 1: STR */
  const boolean check_range;
  const int     min;
  const int     max;
  
  union {
    char str[MAX_PARAM_LENGTH];  // Connie modify, 2014/9/5
    int  val;
  } u;
} request_param_type;

// Connie add start, 2014/9/8
typedef struct {
  char         *name;
  const int     type;  /* 0: INT; 1: STR */
  
  union {
    char str[MAX_PARAM_LENGTH];
    int  val;
  } u;
} response_param_type;

// Connie add end, 2014/9/8
request_param_type json_set_login_params[] = {
// Connie modify start, 2014/9/5
  {"UserName",     PARAM_STR, FALSE,  0, 0, ""},
  {"Password",   PARAM_STR, FALSE, 0, 0, ""},
// Connie modify end, 2014/9/5
};

// Connie add start, 2014/9/5
request_param_type json_set_passwd_params[] = {
  {"UserName",     PARAM_STR, FALSE,  0, 0, ""},
  {"CurrPassword",   PARAM_STR, FALSE, 0, 0, ""},
  {"NewPassword",   PARAM_STR, FALSE, 0, 0, ""},
};

request_param_type json_init_passwd_params[] = {
  {"password",     PARAM_STR, FALSE,  0, 0, ""},
};


response_param_type json_get_login_state_params[] = {
  {"State",     PARAM_INT, ""},
  {"LoginRemainingTimes",     PARAM_INT, ""},
  {"LockedRemainingTime",     PARAM_INT, ""},
  {"PwEncrypt",               PARAM_INT, ""}
};
// Connie add end, 2014/9/5

request_param_type tn_set_login_params[] = {
  {"username",     PARAM_STR, FALSE,  0, 0, ""},
  {"password",   PARAM_STR, FALSE, 0, 0, ""},
};

request_param_type tn_set_passwd_params[] = {
  {"newPassword",   PARAM_STR, FALSE, 0, 0, ""},
  {"oldPassword",   PARAM_STR, FALSE, 0, 0, ""},
};

request_param_type tn_set_password_save_info_params[] = {
  {"username",     PARAM_STR, FALSE, 0, 0, ""},
  {"password",     PARAM_STR, FALSE, 0, 0, ""},
  {"save_flag",    PARAM_INT, FALSE, 0, 0, ""},
};

request_param_type json_set_log_level_params[] = {
  {"websloglevel",     PARAM_STR,FALSE, 0, 0, ""},
};
  /////////////////////add begin by wei.huang.sz/////////////////
response_param_type json_set_password_save_info_params[] = {
  {"username",    PARAM_STR, ""},
  {"password",     PARAM_STR, ""},
  {"save_flag",     PARAM_INT, ""},
};

request_param_type json_ui_pwstate_params[] = {
  {"UIPwState",     PARAM_INT, FALSE,  0, 0, ""},
};

response_param_type json_ui_pwstate_response_params[] = {
  {"UIPwState",    PARAM_INT, ""},
};

response_param_type json_ui_token_response_params[] = {
  {"token",    PARAM_INT, ""},
};

request_param_type json_ui_wifi_pwstate_params[] = {
  {"wifipasswordstate",     PARAM_INT, FALSE,  0, 0, ""},
};

response_param_type json_ui_wifi_pwstate_response_params[] = {
  {"wifipasswordstate",    PARAM_INT, ""},
};

response_param_type json_ui_pwstate_change_flag_response_params[] = {
  {"change_flag",    PARAM_INT, ""},
};

request_param_type json_ui_pwstate_change_flag_params[] = {
  {"change_flag",     PARAM_INT, FALSE,  0, 0, ""},
};
  /////////////////////add end by wei.huang.sz/////////////////
#define GET_ARG(arg)                            \
  websGetVar(wp, arg, "")

#define VALID_ARG(arg)                          \
  ((arg != NULL) ?                              \
    ((strlen(arg) != 0) ? TRUE : FALSE) : FALSE)

#define VALID_INT(arg)                          \
  (arg != INVALID_VALUE ? TRUE : FALSE)

#define IN_RANGE(arg, min, max) \
  (((arg >= min) && (arg <= max)) ? TRUE : FALSE)

#define CHECK_PARAMS(params) \
  webs_check_params(wp, params, COUNTOF(params))

static void print_webs_config(void)
{
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.username        = %s\n",   g_webs_data.username);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.password        = %s\n",   g_webs_data.password);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.lock_web_ui     = %d\n",   g_webs_data.lock_web_ui);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.max_retry_times = %d\n",   g_webs_data.max_retry_times);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.unlock_time     = %d\n",   g_webs_data.unlock_time);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.remember_usrpwd = %d\n",   g_webs_data.remember_usrpwd);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.usrpwd_changed  = %d\n",   g_webs_data.usrpwd_changed);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.free_access     = %d\n",   g_webs_data.free_access);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.webs_security = %d\n",   g_webs_data.webs_security);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.webs_security_nologin  = %d\n",   g_webs_data.webs_security_nologin);  
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_webs_data.webs_unconnected_redirect  = %d\n",   g_webs_data.webs_unconnected_redirect); 
}
int webs_check_sqlite_status(void)
{
  char cmd[128]={0};
  char ret[64]={0};
  FILE * fp;

  if (0 != access("/jrd-resource/resource/sqlite3/user_info.db3",F_OK))
  {
  	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "/jrd-resource/resource/sqlite3/user_info.db3 NOT exists sleep\n");
	return -1;
  }
  snprintf(cmd,sizeof(cmd)-1,"sqlite3 /jrd-resource/resource/sqlite3/user_info.db3 \"PRAGMA integrity_check\"");
 // snprintf(cmd,sizeof(cmd)-1,"sqlite3 /jrd-resource/resource/sqlite3/user_info.db3 \"select * from webs_config \"");
  if(NULL != (fp=popen(cmd,"r")))
  {
    while (fgets(ret, sizeof(ret)-1, fp) != NULL)
    {
      if (ret[strlen(ret)-1] == '\n') 
      {
         ret[strlen(ret)-1] = '\0';
      }
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "sqlite3 check status ret=:%s\n", ret);
      if (NULL != strstr(ret,"ok"))
      {
        return 0;
      }
    }
	pclose(fp);
  }
  return -1; 	
}
/* BEGIN: Added by yuan.jiang, 2019/5/7 */
static char admin_dic[]=
{'3','4','6','7','9','a','b','c','d','e','f','g',
'h','i','k','m','n','p','q','r','t','u','v','w',
'x','y'};

static unsigned long long randseed;
static void jrd_rand_init_seed(char* seed)
{
    randseed = atoll(seed);
}

static unsigned long long jrd_rand(void)
{
    unsigned long long r;
    
    /*basic the imei, return a random num in unsigned long long type that 64 bits*/
    r = randseed = randseed * 9223 + 12345678;
    return (r << 32) | ((r >> 32) & 0xFFFFFFFF);
}

#define MD5_SIZE 16
int compute_md5(char* source_stri, char *md5_str)  
{  
    int i;  
    unsigned char md5_value[MD5_SIZE];  
    struct MD5Context md5;  
  
    // init md5  
    MD5Init(&md5);  
  
    MD5Update(&md5, source_stri, strlen(source_stri));
    
    MD5Final(md5_value, &md5);  
  
    for(i = 0; i < MD5_SIZE; i++)  
    {  
        snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);  
    }
    md5_str[MD5_SIZE*2] = '\0'; // add end
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "md5_str:%s\n",md5_str);
    return 0;  
}  

static int jrd_get_hub_random_string_algo(char *rand_str, int rand_str_len, char *imei, char *dic, int dic_size)
{
    int i, idx;
    if(rand_str == NULL || imei == NULL || dic == NULL) return -1;
    
    /*init the rand_seed by imei*/ 
    jrd_rand_init_seed(imei);
    
    for(i=0; i<rand_str_len; i++)
    {
        idx = jrd_rand() % dic_size;
        rand_str[i] = dic[idx];
        
    /*avoid rand_str include the same words be neighbor like "aa" "33" */    
        if(i != 0 && rand_str[i-1] == rand_str[i]) 
            rand_str[i] = dic[(idx+1)%dic_size]; //make the dic in cyclic use 
    }

    /*the rand_str buff len must larger than the rand_str_len*/
    rand_str[rand_str_len] = '\0';
    return 0;
}

static int webs_get_imei_from_file(char* imei)
{
    FILE *fp;
    int i=0;
    /* BEGIN: Added by yuan.jiang, 2019/8/23 */
    char *imei_f = JRD_DEV_IMEI_FILE;

    if (access(imei_f, F_OK) != 0) {
        return -2;
    }
    /* END:   Added by yuan.jiang, 2019/8/23*/

    do{
      fp=fopen(imei_f, "r");
      if(fp != NULL) break;
    
      if(i<4)
        sleep(3);
      else
        return -2;
      
      i++;
    }while(1);//waiting for core_app write imei file

    fgets(imei,32,fp);
    
    return fclose(fp);
}

static int webs_custom_pwd_implement(void)
{
  long long imei;
  int first_position,second_position;
  char imei_num[32] = {0};
  int flag;
  char cmd[256] = {0};

  
  if(E_WEBS_PASSWORD_TYPE_DEFAULT < g_webs_data.password_type &&
    g_webs_data.password_type < E_WEBS_PASSWORD_TYPE_MAX)
 {
    switch(g_webs_data.password_type)
    {
      case E_WEBS_PASSWORD_TYPE_RDM_4_5_WORDS:
        /* BEGIN: Added by yuan.jiang, 2019/8/23 PN:webs password genreate when 1st startup */
        while (1) {
            if (access(JRD_WEBS_PWD_FLAG, F_OK) != 0) {
                if(webs_get_imei_from_file(imei_num) == -2) {
                    //strcpy(imei_num,"111111111111111");
                    continue;
                }
            } else {
                break;
            }

            if(imei_num[0] == '\0') {
                //strcpy(imei_num,"111111111111111");
                continue;
            } else {
                sprintf (cmd, "touch %s", JRD_WEBS_PWD_FLAG);
                system (cmd);
                break;
            }
        }
        /* END:   Added by yuan.jiang, 2019/8/23*/

        jrd_get_hub_random_string_algo(g_webs_data.password, ADMIN_ORANGE_RANDOM_PSWD_LEN, imei_num, admin_dic, sizeof(admin_dic)/sizeof(admin_dic[0]));
        strncpy(g_webs_data.original_password, g_webs_data.password, ADMIN_ORANGE_RANDOM_PSWD_LEN+1);

        JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"[WEBS]'judge the trace file 305 bytes == 0x03?' result is %d.imei is %s.admin password custom as:%s\n",flag,imei_num,g_webs_data.password); 

        g_webs_data.password_type = E_WEBS_PASSWORD_TYPE_DEFAULT;
        webs_update_database(E_WEBS_PARAM_PASSWORD);
        webs_update_database(E_WEBS_PARAM_ORIGINAL_PASSWORD);
        webs_update_database(E_WEBS_PARAM_PASSWORD_TYPE);
        break;
      default:
        break;
    }
    compute_md5(g_webs_data.password, g_webs_data.password_md5);
    return 0;
 }

 return 0;
    
}

static int webs_custom_config_implement(void)
{
  webs_custom_pwd_implement();
  return 0;
}
/* END:   Added by yuan.jiang, 2019/5/7*/
int webs_init_database(void)
{
  int rc;
  memset(&g_webs_data, 0, sizeof(g_webs_data));
  g_webs_data.webs_unconnected_redirect = 1;


#if 0
  rc = jrd_db_query_db_init_struct(&jrd_webs_config_data_table_info);
  if (rc != 0) {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Error, webs init config data failed. rc:%d\n", rc);
      //return -1;
  }
#else
  int i=0;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "webs init configtimes :%d\n", i);

  while (0 != webs_check_sqlite_status())
  {
    i++;
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "webs init configtimes :%d\n", i);

    sleep(2);
  }
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "webs init configtimes :%d\n", i);


  rc = jrd_db_query_db_init_struct(&jrd_webs_config_data_table_info);
  if (rc != 0) {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Error, webs init config data failed. rc:%d\n", rc);
    //return -1;
  }	


#endif

  rc = jrd_db_query_db_init_struct(&jrd_webs_info_data_table_info);
  if (rc != 0) {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Error, webs init info data failed. rc:%d\n", rc);
      //return -1;
  }

  print_webs_config();
    /* BEGIN: Added by yuan.jiang, 2019/5/7 */
    webs_custom_config_implement();
    /* END:   Added by yuan.jiang, 2019/5/7*/
  return rc;
}
int if_support_connect_redirect(void)
{
  return g_webs_data.support_connet_redirect;
}

char* get_connect_redirect_page(void)
{
  return g_webs_data.connect_redirect_page;
}
int webs_update_database(e_jrd_webs_param_id param_id)
{
  int rc;

  if (param_id == E_WEBS_PARAM_USRPWD_CHANGED)
    rc = jrd_db_update_db_from_struct(&jrd_webs_info_data_table_info, param_id);
  else
    rc = jrd_db_update_db_from_struct(&jrd_webs_config_data_table_info, param_id);
  return rc;
}

// Connie modify start, 2014/9/5
static boolean tn_webs_check_params(webs_t wp, request_param_type *params, int count)
{
  int  i;
  int  val;
  char *arg;

  for (i = 0; i < count; i ++) {
    arg = GET_ARG(params[i].name);
    if (!VALID_ARG(arg)) {
      return FALSE;
    }
    
    if (params[i].type == PARAM_INT) {
      val = atoi(arg);
      if (!VALID_INT(val)) {
        return FALSE;
      }

      if (params[i].check_range) {
        if (!IN_RANGE(val, params[i].min, params[i].max)) {
          return FALSE;
        }
      }

      params[i].u.val = val;
    } else if (params[i].type == PARAM_STR) {
      //params[i].u.str = arg;
      strncpy(params[i].u.str,arg,MAX_PARAM_LENGTH);
    } else {
      return FALSE;
    }
  }

  return TRUE;
}

static boolean json_webs_check_params(webs_t wp, request_param_type *params, int count)
{
  int  i;
  json_object * object_post_data = NULL;
  json_object * object_params = NULL;
  json_object * object_one_param = NULL;

  object_post_data = json_tokener_parse(wp->postData);
  if( is_error(object_post_data) ) 
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "parser data fail!\n");
    return FALSE;
  }
    
  object_params = json_object_object_get(object_post_data, "params");
  if(object_params == NULL) 
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Can't get object_params\n");
    json_object_put(object_post_data);
    return FALSE;
  }
  
  for (i = 0; i < count; i ++)
  {
    object_one_param = json_object_object_get(object_params, params[i].name);
    if(object_one_param == NULL)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Can't get object_one_param\n");
      json_object_put(object_post_data);
      return FALSE;
    }
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Not support getting other type param now, add further if needed! %d %d\n",
	  	json_object_get_type(object_one_param),params[i].type);
    if (params[i].type == PARAM_STR && json_object_get_type(object_one_param) == json_type_string)
    {
      strncpy(params[i].u.str,json_object_get_string(object_one_param),MAX_PARAM_LENGTH);
    }
  /////////////////////add begin by wei.huang.sz/////////////////
    else if (params[i].type == PARAM_INT && json_object_get_type(object_one_param) == json_type_int)
    {
      //strncpy(params[i].u.str,json_object_get_string(object_one_param),MAX_PARAM_LENGTH);
      params[i].u.val = json_object_get_int(object_one_param);
    }
  /////////////////////add end by wei.huang.sz/////////////////
    else
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Not support getting other type param now, add further if needed!\n");
      json_object_put(object_post_data);
      return FALSE;
    }
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "param%d: %s\n",i, params[i].u.str);
  }
  json_object_put(object_post_data);
  return TRUE;
}

boolean json_webs_get_id(webs_t wp, char *id, int id_len)
{
  json_object * object_post_data = NULL;
  json_object * object_id = NULL;

  object_post_data = json_tokener_parse(wp->postData);
  if( is_error(object_post_data) ) 
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "parser data fail!\n");
    return FALSE;
  }
    
  object_id = json_object_object_get(object_post_data, "id");
  if(object_id == NULL) 
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Can't get object_id\n");
    json_object_put(object_post_data);
    return FALSE;
  }
  
  strncpy(id,json_object_get_string(object_id),id_len);
  
  json_object_put(object_post_data);
  return TRUE;
}
// Connie modify end, 2014/9/5
/*****************************************************************************/

void set_timer_clean(void)
{
   //webs_killtimer(&login_check_timer);
   jrd_timer_stop(&login_check_timer);  //Connie modified, 2016/02/25
}

void webstnHeader(webs_t wp)
{
    a_assert(websValid(wp));

    websWrite(wp, T("HTTP/1.0 200 OK\n"));
    websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);
    
    websWrite(wp, T("X-Frame-Options: SAMEORIGIN\n"));
    websWrite(wp, T("x-xss-protection: 1; mode=block\n"));

    websWrite(wp, T("Pragma: no-cache\n"));
    websWrite(wp, T("Cache-control: no-cache\n"));
    websWrite(wp, T("Content-Type: text/html\n"));
    websWrite(wp, T("\n"));
}

#ifdef JRD_EE_CUST_FEATURE_DEVINFO
void websJsonHeader_ee_cust(webs_t wp)
{
    a_assert(websValid(wp));

    websWrite(wp, T("HTTP/1.0 200 OK\n"));
    websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);
    
    websWrite(wp, T("X-Frame-Options: SAMEORIGIN\n"));
    websWrite(wp, T("x-xss-protection: 1; mode=block\n"));

    websWrite(wp, T("Pragma: no-cache\n"));
    websWrite(wp, T("Cache-control: no-cache\n"));
    websWrite(wp, T("Content-Type: text/html\n"));
    websWrite(wp, T("Access-Control-Allow-Origin: http://myaccount.ee.co.uk http://my.ee.co.uk http://add-on.ee.co.uk http://ee.co.uk\n"));
    websWrite(wp, T("\n"));
}
#endif


void websJsonHeader(webs_t wp, uint32 content_len, e_jrd_cust_flag_type flag_type)
{
    a_assert(websValid(wp));
    websWrite(wp, T("HTTP/1.1 200 OK\n"));
    if (flag_type == E_CUST_JSON_RESTORE)
      websWrite(wp, T("Content-Type: text/html\n"));
    else
      websWrite(wp, T("Content-Type: application/json\n"));
      
    websWrite(wp, T("X-Frame-Options: SAMEORIGIN\n"));
    websWrite(wp, T("x-xss-protection: 1; mode=block\n"));
    
    websWrite(wp, T("Content-Length: %d\n"), content_len);
    websWrite(wp, T("\n"));
}
static void tn_webs_post_set_return_data(webs_t wp,int set_res)
{
   webstnHeader(wp);
   websWrite(wp, T("{\"error\":%d}"),set_res);
   websDone(wp, 200);
}

static int webs_get_server_info(int eid, webs_t wp, int argc, char_t **argv)
{
  char *operate_key = NULL;
  char *current_lang = NULL;
  int login_status = 0;
  
  operate_key = websGetVar(wp, T("key"), T(""));
  
  if((operate_key != NULL)&&(0 == strcmp(operate_key,"language")))
  {
	  return websWrite(wp, T("{\"language\":\"%s\",\"error\":%d}"),"en",WEBS_SET_SUCCESS);//***no use
  }
  else
  { 
	  login_status = ASPE_getSessionID(wp);
     return websWrite(wp, T("{\"loginStatus\":%d,\"error\":%d}"),login_status,WEBS_SET_SUCCESS);
  }
}

char* jrd_encrypt(char* source, char* pass)
{
    int source_length = strlen(source);
    int pass_length = strlen(pass);
    int i = 0;
 		
 		int key_len = strlen(jrd_key);
    char* tmp_str = (char*)malloc(2*(source_length + 1) * sizeof(char));
    memset(tmp_str, 0, 2*(source_length + 1) * sizeof(char));
 
    /*for(i = 0; i < source_length; i++)
    {
        tmp_str[2*i] = (0b0011<<4)|(source[i]&0xf);
        tmp_str[2*i+1] = (0b0110<<4)|(source[i]>>4);
    }*/
    
    /*for(i = 0; i < source_length; i++)
    {
        tmp_str[2*i] = (jrd_key[i%key_len]&0xf0)|(source[i]&0xf);
        tmp_str[2*i+1] = (jrd_key[i%key_len]&0xf0)|(source[i]>>4);
        printf("%02x:%02x,",tmp_str[2*i],tmp_str[2*i+1]);
    }*/
    
    for(i = 0; i < source_length; i++)
    {
        tmp_str[2*i] = (jrd_key[i%key_len]&0xf0)|((source[i]&0xf)^(jrd_key[i%key_len]&0xf));
        tmp_str[2*i+1] = (jrd_key[i%key_len]&0xf0)|((source[i]>>4)^(jrd_key[i%key_len]&0xf));
        //printf("%02x:%02x,",tmp_str[2*i],tmp_str[2*i+1]);
    }
    
 
 		/*for(i = 0; i < source_length; i++)
    {
    		if(i+1 >= source_length)
    		{
    				tmp_str[i] = (0b0011<<4)|(source[i]&0xf);
        		tmp_str[2*i] = (0b0110<<4)|(source[i]>>4);
    		}
    		else
    		{
	        tmp_str[i] = (source[i]&0xf0)|(source[i+1]&0x0f);
	        tmp_str[i+1] = (source[i+1]&0xf0)|(source[i]&0x0f);
	        i++;
      	}
    }*/
    
    /*for(i = 0; i < source_length; i++)
    {
        tmp_str[i] = (source[i]&0xf0)|((source[i]&0xf)^(jrd_key[i%key_len]&0xf));
        //tmp_str[2*i] = (0b0110<<4)|(source[i]>>4);
    }*/
    
     /*for(i = 0; i < source_length; i++)
    {
        tmp_str[i] = source[i]^(jrd_key[i%key_len]&0xf);
        //printf("%x,",tmp_str[i]);
        if(tmp_str[i] == 0)              
        {                            
            tmp_str[i] = source[i];
        }
    }*/
    
    return tmp_str;
}

char* jrd_decrypt(char* source)
{
    if(!source)
      return NULL;
    int source_length = strlen(source);
    int pass_length = strlen(jrd_key);
    int i = 0,new_i = 0;
 		
 		int key_len = strlen(jrd_key);
    char* tmp_str = (char*)malloc((source_length + 1) * sizeof(char));
    memset(tmp_str, 0, (source_length + 1) * sizeof(char));
   
    for(i = 0; i < source_length; i++)
    {
    		if(i == 0)
  			{
  				new_i = 0;
  			}
  			else
				{
					new_i=i/2;
				}
        tmp_str[new_i] = ((source[i+1]^jrd_key[new_i%key_len])<<4)|((source[i]^jrd_key[new_i%key_len])&0xf);
        //printf("%02x,",tmp_str[new_i]);
        i++;
    }
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "tmp_str:%s\n",tmp_str);
    return tmp_str;
}

static boolean hasUserLogined(char * pstrIp, int *pnSessionID)  // Connie modify, 2014/9/5
{
	
	if(ASPE_getFirstSessionInfo(pstrIp, pnSessionID) == 1) // Connie modify, 2014/9/5
	{
		if(*pnSessionID == 5)
		{
			return TRUE;
		}
	}
	else 
	{
		return FALSE;
	}
		
	while( ASPE_getNextSessionInfo(pstrIp, pnSessionID)== 1) // Connie modify, 2014/9/5
	{
		if(*pnSessionID == 5)
		{
			return TRUE;
		}
	}
			
  return FALSE;
}
// zengyong add, 2015/3/4
void *Login_fail_time_count (void *data)
{
  int i = 0;
  g_login_fail_time_count = 0;
  
  for (i=0; i<g_webs_data.unlock_time; i++)
  {
    sleep(1);
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Login_fail_time_count sleep!\n");
    g_login_fail_time_count++;	
  }

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Login times remain %d\n", g_login_fail_time_count);
  g_login_fail_count = 0;  //for recount
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "g_login_fail_count = %d\n",g_login_fail_count);
  pthread_exit(NULL);
}
// Connie add start, 2014/9/5
static json_object* json_webs_create_error_object(int error_code)
{
  json_object * err_obj = NULL;

  err_obj = json_object_new_object();
  if (err_obj == NULL)
    return NULL;
  
  switch (error_code)
  {
    case 10101:
      json_object_object_add(err_obj, "code", json_object_new_string("010101"));
      json_object_object_add(err_obj, "message", json_object_new_string("Username or Password is not correct."));
      break;
    case 10102:
      json_object_object_add(err_obj, "code", json_object_new_string("010102"));
      json_object_object_add(err_obj, "message", json_object_new_string("Other user is login."));
      break;
    case 10103:
      json_object_object_add(err_obj, "code", json_object_new_string("010103"));
      json_object_object_add(err_obj, "message", json_object_new_string("Login times is used out."));
      break;
    case 10201:
      json_object_object_add(err_obj, "code", json_object_new_string("010201"));
      json_object_object_add(err_obj, "message", json_object_new_string("Logout failed."));
      break;
    case 10301:
      json_object_object_add(err_obj, "code", json_object_new_string("010301"));
      json_object_object_add(err_obj, "message", json_object_new_string("Get login state failed."));
      break;
    case 10401:
      json_object_object_add(err_obj, "code", json_object_new_string("010401"));
      json_object_object_add(err_obj, "message", json_object_new_string("Change password failed."));
      break;
    case 10402:
      json_object_object_add(err_obj, "code", json_object_new_string("010402"));
      json_object_object_add(err_obj, "message", json_object_new_string("The current password is wrong."));
      break;
     case 10403:
      json_object_object_add(err_obj, "code", json_object_new_string("010403"));
      json_object_object_add(err_obj, "message", json_object_new_string("The current password is the same as default password."));
      break;
    case 10501:
      json_object_object_add(err_obj, "code", json_object_new_string("010501"));
      json_object_object_add(err_obj, "message", json_object_new_string("This method is wrong."));
      break;
    case 10601:
      json_object_object_add(err_obj, "code", json_object_new_string("010601"));
      json_object_object_add(err_obj, "message", json_object_new_string("Username or Password is not correct."));
      break;
    case 10602:
      json_object_object_add(err_obj, "code", json_object_new_string("010602"));
      json_object_object_add(err_obj, "message", json_object_new_string("Login times is used out. "));
      break;
  /////////////////////add begin by wei.huang.sz/////////////////
    case 11201:
      json_object_object_add(err_obj, "code", json_object_new_string("011201"));
      json_object_object_add(err_obj, "message", json_object_new_string("Set UIPwState Failed. "));
      break;
    case 11101:
      json_object_object_add(err_obj, "code", json_object_new_string("011101"));
      json_object_object_add(err_obj, "message", json_object_new_string("Get UIPwState Failed. "));
      break;
    case 11001:
      json_object_object_add(err_obj, "code", json_object_new_string("011001"));
      json_object_object_add(err_obj, "message", json_object_new_string("Set PasswordSaveInfo Failed."));
      break;
    case 10901:
      json_object_object_add(err_obj, "code", json_object_new_string("010901"));
      json_object_object_add(err_obj, "message", json_object_new_string("get PasswordSaveInfo Failed."));
      break;
  /////////////////////add end by wei.huang.sz/////////////////
    case -32700:
      json_object_object_add(err_obj, "code", json_object_new_string("-32700"));
      json_object_object_add(err_obj, "message", json_object_new_string("Parse error"));
      break;
    case WEBS_REFERER_ERROR:
      json_object_object_add(err_obj, "code", json_object_new_string("-32697"));
      json_object_object_add(err_obj, "message", json_object_new_string("Authentication Failure"));
      break;
    case WEBS_TOKEN_ERROR:
      json_object_object_add(err_obj, "code", json_object_new_string("-32699"));
      json_object_object_add(err_obj, "message", json_object_new_string("Authentication Failure"));
      break;

    case WEBS_NEED_LOGIN:
      json_object_object_add(err_obj, "code", json_object_new_string("-32698"));
      json_object_object_add(err_obj, "message", json_object_new_string("Request need login"));
      break;
    
    case 012101:
      json_object_object_add(err_obj, "code", json_object_new_string("012101"));
      json_object_object_add(err_obj, "message", json_object_new_string("Init password failed"));
      break;
      
    default:
      json_object_object_add(err_obj, "code", json_object_new_string("-1"));
      json_object_object_add(err_obj, "message", json_object_new_string("failed"));
  }
  return err_obj;
}

static void json_webs_rsp_result(webs_t wp, char_t *id, response_param_type *rsp_params, int count) // Connie modify, 2014/9/8
{
  json_object * rsp_object = NULL;
  json_object * result_object = NULL;
  char*         format_data_buf = NULL;
  int           i;

  if (id == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "id is NULL!\n");
    return;
  }
  rsp_object = json_object_new_object();
  if (rsp_object == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Create rsp_object fail!\n");
  }
  else
  {
    result_object = json_object_new_object();
    if (result_object == NULL)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Create result_object fail!\n");
    }
    else
    {
	  // Connie add start, 2014/9/8
      if (rsp_params && count>0)
      {
        for (i = 0; i < count; i++)
        {
          if (rsp_params[i].type == PARAM_STR)
          {
            json_object_object_add(result_object, rsp_params[i].name, json_object_new_string(rsp_params[i].u.str));
          }
          else if (rsp_params[i].type == PARAM_INT)
          {
            json_object_object_add(result_object, rsp_params[i].name, json_object_new_int(rsp_params[i].u.val));
          }
          else
          {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Invalid response parameter type!\n");
          }
        }      
      }
	  // Connie add end, 2014/9/8
      json_object_object_add(rsp_object, "jsonrpc", json_object_new_string("2.0"));
      json_object_object_add(rsp_object, "result", result_object);
      json_object_object_add(rsp_object, "id", json_object_new_string(id));
      format_data_buf = json_object_to_json_string(rsp_object);

      websJsonHeader(wp, gstrlen(format_data_buf), E_CUST_DEFAULT);
      websWrite(wp, format_data_buf);
      websDone(wp, 200);
    }
    json_object_put(rsp_object);
  }
}

void json_webs_rsp_err(webs_t wp, int error_code, char_t *id)
{
  json_object * rsp_object = NULL;
  json_object * error_object = NULL;
  char*         format_data_buf = NULL;

  if (id == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "id is NULL!\n");
    return;
  }
  
  rsp_object = json_object_new_object();
  if (rsp_object == NULL)
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Create rsp_object fail!\n");
  }
  else
  {
    error_object = json_webs_create_error_object(error_code);
    if (error_object == NULL)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Create error_object fail!\n");
    }
    else
    {
      json_object_object_add(rsp_object, "jsonrpc", json_object_new_string("2.0"));
      json_object_object_add(rsp_object, "error", error_object);
      json_object_object_add(rsp_object, "id", json_object_new_string(id));
      format_data_buf = json_object_to_json_string(rsp_object);

      websJsonHeader(wp, gstrlen(format_data_buf), E_CUST_DEFAULT);
      websWrite(wp, format_data_buf);
      websDone(wp, 200);
    }
    json_object_put(rsp_object);
  }
}
// Connie add end, 2014/9/5

void tn_webs_get_login_state(webs_t wp, char_t *path, char_t *query)
{
  int login_status = 0;
				
	login_status = ASPE_getSessionID(wp);
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "login state:%d\n",login_status);

	webstnHeader(wp);
  websWrite(wp, T("{\"loginStatus\":%d,\"error\":%d}"),login_status,WEBS_SET_SUCCESS);
  websDone(wp, 200);
}
  /////////////////////add begin by wei.huang.sz/////////////////
static timer_info_t timer_check_login_time = {0};  //Connie modified, 2016/03/23
static int login_timer_start = 0;
static int jrd_oem_webui_check_time(void *ind_info)
{

       json_get_login_state_params[2].u.val ++;
       if(json_get_login_state_params[2].u.val > g_webs_data.webs_recover_time)
       {
          jrd_timer_stop(&timer_check_login_time);
          g_login_fail_count  = 0;
          login_timer_start = 0;
          json_get_login_state_params[2].u.val = 0;
       }

       JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW,"jrd_oem_statistics_overtime_ind() %d\n",json_get_login_state_params[2].u.val);
	return 0;
}
/////////////////////add end by wei.huang.sz/////////////////
void json_webs_get_login_state(webs_t wp, char_t *path, char_t *query)
{
  int login_status = 0;
  int login_fail_cnt = 0;
  char id[MAX_ID_LENGTH] = {0};
  int rc = 0;

  if(E_WEBS_UI_PW_NOT_NEEDED == g_webs_data.webs_uipwstate)
  {
    login_status = E_WEBS_LOGIN_STATE_LOGIN;
    goto send_respons;
  }
  
	login_status = ASPE_getSessionID(wp);
  if (login_status == SESSIONID_FOR_LOGIN_SUCCESS) // login
    login_status = E_WEBS_LOGIN_STATE_LOGIN;
  else   // logout
  {
   // login_fail_cnt = ASPE_getLoginFailCnt(wp);
    if (g_login_fail_count >= g_webs_data.max_retry_times)
      login_status = E_WEBS_LOGIN_STATE_USED_OUT; //the login times used out.
    else
      login_status = E_WEBS_LOGIN_STATE_LOGOUT; //logout
  }
  /////////////////////add begin by wei.huang.sz/////////////////  
  if((g_login_fail_count >= g_webs_data.max_retry_times) && (0  != g_webs_data.webs_recover_time)
  	&& (0 == login_timer_start))
  {
    login_timer_start = 1;
    //timer_check_usage = jrd_timer_start(jrd_oem_check_overtime_by_time, &ts, &reload_ts, NULL);
    timer_check_login_time.ts.tv_sec = 1;
    timer_check_login_time.ts_interval.tv_sec = 1;
    timer_check_login_time.cb.handler = jrd_oem_webui_check_time;
    timer_check_login_time.cb.params = NULL;
    rc = jrd_timer_start(&timer_check_login_time);
    if(rc) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Failed to start usage conn status timer(%dS, %dnS), rc=%d\n", 
            timer_check_login_time.ts.tv_sec, timer_check_login_time.ts.tv_nsec, rc);
    }
  }
send_respons:
  /////////////////////add end by wei.huang.sz/////////////////
  json_webs_get_id(wp, id, MAX_ID_LENGTH);
  json_get_login_state_params[0].u.val = login_status;
  json_get_login_state_params[1].u.val = g_webs_data.max_retry_times - g_login_fail_count;
  json_get_login_state_params[3].u.val = g_webs_data.webs_pw_username_encrypt;
  json_webs_rsp_result(wp, id, json_get_login_state_params, sizeof(json_get_login_state_params)/sizeof(json_get_login_state_params[0]));
}

void tn_webs_get_wlan_info(webs_t wp, char_t *path, char_t *query)
{
	webstnHeader(wp);
  websWrite(wp, T("{ \"wifi_state\": 1, \"wlan_mode\": 0, \"ssid_2_state\": 0, \"ssid_2\": \"MY WIFI _5G\", \"ssid_5g\": \"MY WIFI _5G\", \"hidden_ssid_2\": 0, \"hidden_ssid_5g\": 0, \"max_numsta_2\": 15, \"max_numsta_5g\": 15, \"security_mode_2\": 3, \"wep_sec_2\": 0, \"wep_key_2\": \"1234567890\", \"wpa_sec_2\": 2, \"curr_num_2\": 0, \"wpa_passphrase_2\": \"MYWIFI\", \"ap_status_2\": 0, \"wmode\": 2, \"wmode_5g\": 2, \"ssid\": \"MY WIFI \", \"hidden_ssid\": 0, \"channel\": 0, \"channel_5g\": 0, \"max_numsta\": 15, \"curr_num\": 0, \"security_mode\": 3, \"wep_sec\": 0, \"wep_key\": \"1234567890\", \"wpa_sec\": 2, \"wpa_passphrase\": \"MYWIFI\", \"wifi_country_code\": \"CN\", \"ap_status\": 0, \"error\": 0 }"));
  websDone(wp, 200);
}

void tn_webs_get_wan_info(webs_t wp, char_t *path, char_t *query)
{
	webstnHeader(wp);
  websWrite(wp, T("{ \"wan_state\": 0, \"wan_ip\": \"0.0.0.0\", \"wan_ip6\": \"0::0\", \"network_type\": -1, \"network_name\": \"N\/A\", \"roam\": 255, \"dur_time\": 126, \"download\": 42200000, \"upload\": 5742000, \"usage\": 21066, \"error\": 0 }"));
  websDone(wp, 200);
}


void tn_web_save_ipaddr(const char *ip)
{
	int len = strlen(ip);
	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "log in success : %s, len:%d\n", ip, len);

	FILE *fp = fopen("/var/run/loginip", "w");
	if (fp != NULL)
	{
		fwrite(ip, 1, len, fp);
		fclose(fp);
	}
	
	//char cmd[128];
	//sprintf(cmd, 128, "echo \"%s\" > /var/run/loginip", ip);
	//system(cmd);
}


// zengyong add, 2015/3/4
static webs_login_state webs_set_login(webs_t wp, request_param_type* params, boolean blForce)
{
  char ip_browser_str[20] = {0};
  char ip_str[20] = {0};
  int browser_type = 0;
  int session_id = 0;
  int rc;
  //int login_fail_cnt = 0;
  // Connie modify start, 2015/2/10
  //g_login_fail_count = ASPE_getLoginFailCnt(wp);
  if (g_webs_data.lock_web_ui)
  {
    if (g_login_fail_count >= g_webs_data.max_retry_times)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Login times is used out.\n");
      return WEBS_LOGIN_LIMIT;
    } 
  }

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login g_webs_data.username:%s,g_webs_data.password:%s\n",g_webs_data.username,g_webs_data.password);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login username:%s,Password:%s \n",params[0].u.str,params[1].u.str);
  if(0 == strcmp(params[0].u.str, g_webs_data.username) && (0 == strcmp(params[1].u.str, g_webs_data.password)))
  {
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login wrong password!\n");
    //ASPE_userLogin(wp, E_LOGIN_FAIL);//user login fail
    g_login_fail_count++;
    //ASPE_setLoginFailCnt(wp, (g_login_fail_count+1));
    //JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login fail %d times!\n", ASPE_getLoginFailCnt(wp));
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "g_login_fail_count = %d\n",g_login_fail_count);

		if (g_webs_data.lock_web_ui && (g_webs_data.max_retry_times == g_login_fail_count))
		{
		      if (g_webs_data.unlock_time == -1)
		      {
		      	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "unlock_time == -1!return\n");
		      	return WEBS_LOGIN_FAILED;
		      }
		  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "pthread_create!\n");
            	  pthread_create( &g_login_time_thrd, NULL, Login_fail_time_count, NULL);
		}

    return WEBS_LOGIN_FAILED;
    //ASPE_userLogin(wp, 1);//user login fail
  }	


  if(hasUserLogined(ip_browser_str, &session_id) == TRUE)
  {
    char ip_str2[20] = {0};
    char ip_browser_str2[20] = {0};
    int browser_type2 = 0;

    ASPE_getPeerInfo(wp,ip_str2,&browser_type2);
    gsprintf(ip_browser_str2, "%s_%d", ip_str2, browser_type2);

    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ip_browser_str:%s, ip_browser_str2:%s, browser_type2:%d\n",ip_browser_str, ip_browser_str2, browser_type2);
	
    if(strcmp(ip_browser_str,ip_browser_str2) == 0)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Already log in \n");
      return WEBS_LOGIN_ALREADY_LOGIN;
    }
    else if (!blForce)
    //below may cause many unused symbol in session table !
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Other one has logged in \n");
      ASPE_userLogin(wp, E_LOGIN_OTHER_HAS_LOGIN);
      return WEBS_LOGIN_OTHER;
    }
  }
 
  #if 0
    if(0 == strcmp(params[0].u.str, g_webs_data.username) && (0 == strcmp(params[1].u.str, g_webs_data.password)))
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "log in success \n");
      //modify by PiFangsi 2015-06-23 [add force login] start
      if (blForce)
      {
  		  set_timer_clean();
  		  ASPE_userLogin(wp, E_LOGIN_FORCE_LOGIN_SUCCESS);//user force login success	
  	  }
  	  else
  	  {
  		  ASPE_userLogin(wp, E_LOGIN_SUCCESS);//user login success	
  	  }
  	  //modify by PiFangsi 2015-06-23 [add force login] start
      ASPE_getPeerInfo(wp,ip_str,&browser_type);

		//add by yunhua.zhao , save ip address to /var/run/weblogin
		  tn_web_save_ipaddr(ip_str);
		
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ip_str =%s, browser_type=%d\n", ip_str, browser_type);
      ASPE_setLoginFailCnt(wp, 0);


	  
	  
      /*****************************************/
      //start timer to check user

      //Connie modify start, 2016/02/24
      //login_check_timer = webs_regtimer(EGOSM_processSessionTimeOut,10,10,0);
      login_check_timer.ts.tv_sec = 10;
      login_check_timer.ts_interval.tv_sec = 10;
      login_check_timer.cb.handler = EGOSM_processSessionTimeOut;
      login_check_timer.cb.params = NULL;
      rc = jrd_timer_start(&login_check_timer);
      if(rc) {
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Failed to start login check timer(%dS, %dnS), rc=%d\n", 
              login_check_timer.ts.tv_sec, login_check_timer.ts.tv_nsec, rc);
      }
      //Connie modify end, 2016/02/24
      /*******************************************/

	    g_login_fail_count = 0;

      return WEBS_LOGIN_SUCESS;
    }
    else
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login wrong password!\n");
	    ASPE_userLogin(wp, E_LOGIN_FAIL);//user login fail
	    g_login_fail_count++;
      //ASPE_setLoginFailCnt(wp, (g_login_fail_count+1));
      //JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login fail %d times!\n", ASPE_getLoginFailCnt(wp));
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "g_login_fail_count = %d\n",g_login_fail_count);

			if (g_webs_data.lock_web_ui && (g_webs_data.max_retry_times == g_login_fail_count))
			{
			      if (g_webs_data.unlock_time == -1)
			      {
			      	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "unlock_time == -1!return\n");
			      	return WEBS_LOGIN_FAILED;
			      }
			  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "pthread_create!\n");
              	  pthread_create( &g_login_time_thrd, NULL, Login_fail_time_count, NULL);
			}

      return WEBS_LOGIN_FAILED;
      //ASPE_userLogin(wp, 1);//user login fail
    }
  #else
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "log in success \n");
  //modify by PiFangsi 2015-06-23 [add force login] start
  if (blForce)
  {
    set_timer_clean();
    ASPE_userLogin(wp, E_LOGIN_FORCE_LOGIN_SUCCESS);//user force login success  
  }
  else
  {
    ASPE_userLogin(wp, E_LOGIN_SUCCESS);//user login success  
  }
  //modify by PiFangsi 2015-06-23 [add force login] start
  ASPE_getPeerInfo(wp,ip_str,&browser_type);

  //add by yunhua.zhao , save ip address to /var/run/weblogin
  tn_web_save_ipaddr(ip_str);


  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ip_str =%s, browser_type=%d\n", ip_str, browser_type);
  ASPE_setLoginFailCnt(wp, 0);
  /*****************************************/
  //start timer to check user

  //Connie modify start, 2016/02/24
  //login_check_timer = webs_regtimer(EGOSM_processSessionTimeOut,10,10,0);
  login_check_timer.ts.tv_sec = 10;
  login_check_timer.ts_interval.tv_sec = 10;
  login_check_timer.cb.handler = EGOSM_processSessionTimeOut;
  login_check_timer.cb.params = NULL;
  rc = jrd_timer_start(&login_check_timer);
  if(rc) {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Failed to start login check timer(%dS, %dnS), rc=%d\n", 
          login_check_timer.ts.tv_sec, login_check_timer.ts.tv_nsec, rc);
  }
  //Connie modify end, 2016/02/24
  /*******************************************/

  g_login_fail_count = 0;

  return WEBS_LOGIN_SUCESS;
  #endif
}
// zengyong end, 2015/3/4

void tn_webs_set_login(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name = NULL, *password = NULL;
	int set_res = WEBS_SET_SUCCESS;

	char *name_cfg, *password_cfg;
	int session_id = 0;
	webs_login_state login_status = 0;

    char ip_str[20] = {0};
    int browser_type = 0;
    FILE *f;

  if(g_webs_data.webs_security == 1)
  {
    if(wp->token_flag == 0)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_webs_set_login^^^^^\n");
      set_res = 255;
      webstnHeader(wp);
      websWrite(wp, T("{\"login_result\":%d,\"error\":%d}"),login_status,set_res);
      websDone(wp, 200);
      return;
    }
  }
  if (tn_webs_check_params(wp, tn_set_login_params, COUNTOF(tn_set_login_params)))
	{
     login_status = webs_set_login(wp, tn_set_login_params, TRUE);
	}
	else
	{
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "invalid params\n");
	  set_res = WEBS_GET_UIPARA_WRONG;
	}
	
    if (login_status == WEBS_LOGIN_SUCESS)
    {
        ASPE_getPeerInfo(wp,ip_str,&browser_type);
        f = fopen(WLAN_FILE_LOGIN_IP, "w+");
        if(NULL != f)
        {
            fprintf(f, "%s\n", ip_str);
            fclose(f);
        }
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login in %s\n",ip_str);
    }
  webstnHeader(wp);
  websWrite(wp, T("{\"login_result\":%d,\"error\":%d}"),login_status,set_res);
  websDone(wp, 200);
}

  /////////////////////add begin by wei.huang.sz/////////////////
static char itoa(uint8 digit)
{  
  if (digit>15)
    return ' ';

  if (digit<=9)
    return ('0'+digit);
  if (digit >9 && digit <= 15)
    return ('a'+digit-10);
}

void json_webs_set_PasswordSaveInfo(webs_t wp, char_t *path, char_t *query)
{
  // Connie modify start, 2014/9/5
  char id[MAX_ID_LENGTH] = {0};
  char ip_str[20] = {0};
  int browser_type = 0;
  int session_id = 0;
  int error_code = 0;
  int login_ret = 0;
  FILE *f;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "wp->path: %s, path: %s, query:%s\n", wp->path, path, query);

  if (json_webs_check_params(wp, tn_set_password_save_info_params, COUNTOF(tn_set_password_save_info_params)))
  {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_set_password_save_info_params[2].u.val .%d\n",tn_set_password_save_info_params[2].u.val);
        g_webs_data.remember_usrpwd = tn_set_password_save_info_params[2].u.val;//atoi(json_set_password_save_info_params[2].u.str);

        if(g_webs_data.remember_usrpwd)
        {
	        memcpy(g_webs_data.username , tn_set_password_save_info_params[0].u.str, MAX_PARAM_LENGTH);
	        memcpy(g_webs_data.password ,tn_set_password_save_info_params[1].u.str , MAX_PARAM_LENGTH);
	        login_ret = webs_update_database(E_WEBS_PARAM_USER_NAME);
	        login_ret = webs_update_database(E_WEBS_PARAM_PASSWORD);
        }
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "g_webs_data.remember_usrpwd.%d\n",g_webs_data.remember_usrpwd);
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "g_webs_data.username %s\n",g_webs_data.username);
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "g_webs_data.password %s\n",g_webs_data.password);
        login_ret = webs_update_database(E_WEBS_PARAM_REMEMBER_USRPWD);

        if(0 == login_ret)
        {
	      error_code  = WEBS_SET_SUCCESS;
        }
        else
        {
	      error_code  = 11001;
        }
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Get json_webs_set_PasswordSaveInfo params fail.\n");
    error_code = 11001;
  }

    
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "error_code=%d \n", error_code);
  json_webs_get_id(wp, id, MAX_ID_LENGTH);
  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
    json_webs_rsp_result(wp,id, NULL,0);
  
  return;
  // Connie modify start, 2014/9/5
}

void json_webs_get_PasswordSaveInfo(webs_t wp, char_t *path, char_t *query)
{
  char id[MAX_ID_LENGTH] = {0};


  json_webs_get_id(wp, id, MAX_ID_LENGTH);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "tn_set_password_save_info_params[2].u.str = %s\n",tn_set_password_save_info_params[2].u.str);
  //tn_set_password_save_info_params[2].u.str = itoa(g_webs_data.remember_usrpwd);
  //memcpy(tn_set_password_save_info_params[2].u.str, &remember_usrpwd, 1);
  json_set_password_save_info_params[2].u.val = g_webs_data.remember_usrpwd;
  memcpy(json_set_password_save_info_params[0].u.str, g_webs_data.username, MAX_PARAM_LENGTH);
  memcpy(json_set_password_save_info_params[1].u.str, g_webs_data.password, MAX_PARAM_LENGTH);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "tn_set_password_save_info_params[2].u.val = %d\n",json_set_password_save_info_params[2].u.val);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "tn_set_password_save_info_params[0].u.str = %s\n",json_set_password_save_info_params[0].u.str);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "tn_set_password_save_info_params[1].u.str = %s\n",json_set_password_save_info_params[1].u.str);
  json_webs_rsp_result(wp, id, json_set_password_save_info_params,3);
}

/*void json_webs_Set_UIPwState(webs_t wp, char_t *path, char_t *query)
{
  char id[MAX_ID_LENGTH] = {0};
  int error_code = 0;
  FILE *f;
  //char UIPwState = '0';

  if(!wp->token)
  {
    error_code=WEBS_TOKEN_ERROR;
    goto send_respons;
  }
  else if(E_WEBS_UI_PW_NEEDED == g_webs_data.webs_uipwstate && strcmp(jrd_webs_token_str,wp->token)!=0)
  {
    error_code=WEBS_TOKEN_ERROR;
    goto send_respons;
  }
  else if(E_WEBS_UI_PW_NEEDED != g_webs_data.webs_uipwstate 
        && strcmp(jrd_default_token,wp->token)!=0 
        && strcmp(jrd_webs_token_str,wp->token)!=0)
  {
    error_code=WEBS_TOKEN_ERROR;
    goto send_respons;
  }
  
  if (json_webs_check_params(wp, json_ui_pwstate_params, COUNTOF(json_ui_pwstate_params)))
  {
         //UIPwState = itoa(json_ui_pwstate_params[0].u.str);
         JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_webs_Set_UIPwState %d\n",json_ui_pwstate_params[0].u.val);
         g_webs_data.webs_uipwstate =  json_ui_pwstate_params[0].u.val;
         webs_update_database(E_WEBS_PARAM_UIPWSTATE);
#if 0
	  f = fopen("/jrd-resource/resource/uipwstate", "w+");
	  if(NULL !=f)
	  {
		  fprintf(f, "%d\n",json_ui_pwstate_params[0].u.val );
		  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_webs_Set_UIPwState %d\n",json_ui_pwstate_params[0].u.val);
		  fclose(f);
	  }
#endif
  }
  else
  {
      error_code = 11201;
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "error_code=%d \n", error_code);
  }
  
send_respons:

  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
    json_webs_rsp_result(wp,id, NULL,0);
  return;
}*/

void json_webs_get_UIPwState(webs_t wp, char_t *path, char_t *query)
{
  char id[MAX_ID_LENGTH] = {0};
  int error_code = 0;
  FILE *f;
  char UIPwState[2] = {0};
#if 0
  f = fopen("/jrd-resource/resource/uipwstate", "r");
  if( NULL != f)
  {
		  if( NULL != fgets(UIPwState,sizeof(UIPwState),f))
		  {
	                json_ui_pwstate_response_params[0].u.val = atoi(UIPwState);
	                JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "UIPwState=%c \n", UIPwState);
                       JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "UIPwState=%s \n", UIPwState);
		         JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_webs_Get_UIPwState %d\n",json_ui_pwstate_response_params[0].u.val);
		  }
		  else
		  {
		       error_code = 11101;
		       JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "error_code=%d \n", error_code);
		  }
		  fclose(f);
  }
  else
  {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "UIPwState=%s \n", UIPwState);
  }
#endif
  json_ui_pwstate_response_params[0].u.val = g_webs_data.webs_uipwstate;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_webs_Get_UIPwState %d\n",json_ui_pwstate_response_params[0].u.val);
  
  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
  json_webs_rsp_result(wp, id, json_ui_pwstate_response_params,1);
  return;
}

void json_webs_get_UserNameAndPw(webs_t wp, char_t *path, char_t *query)
{
  char id[MAX_ID_LENGTH] = {0};
  int UIPwStat =0;
  char UIPwState[2] = {0};
  FILE *f;
  
  f = fopen("/jrd-resource/resource/uipwstate", "r");
  if( NULL != f)
  {
		  if( NULL != fgets(UIPwState,sizeof(UIPwState),f))
		  {
                json_ui_pwstate_response_params[0].u.val = atoi(UIPwState);
                JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "UIPwState=%c \n", UIPwState);
	         JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_webs_Get_UIPwState %d\n",json_ui_pwstate_response_params[0].u.val);
		  }
		  else
		  {
                   JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "fgets fail \n");
		  }
		  fclose(f);
  }

  json_webs_get_id(wp, id, MAX_ID_LENGTH);
  memcpy(json_set_password_save_info_params[0].u.str,g_webs_data.username,MAX_PARAM_LENGTH);
  memcpy(json_set_password_save_info_params[1].u.str,g_webs_data.password,MAX_PARAM_LENGTH);
  UIPwStat = json_ui_pwstate_response_params[0].u.val;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "UIPwStat=%d \n", UIPwStat);
  if(UIPwStat)
  {
      json_webs_rsp_result(wp, id, json_set_password_save_info_params,2);
  }
  else
  {
      json_webs_rsp_result(wp, id, NULL,0);
  }
  return;
}


  /////////////////////add end by wei.huang.sz/////////////////
void json_webs_set_login(webs_t wp, char_t *path, char_t *query)
{
  // Connie modify start, 2014/9/5
  char id[MAX_ID_LENGTH] = {0};
  char ip_str[20] = {0};
  int browser_type = 0;
  int session_id = 0;
  int error_code = 0;
  webs_login_state login_ret;
  FILE *f;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "wp->path: %s, path: %s, query:%s\n", wp->path, path, query);

  if (json_webs_check_params(wp, json_set_login_params, COUNTOF(json_set_login_params)))
  {
    login_ret = webs_set_login(wp, json_set_login_params, TRUE);
    if (login_ret == WEBS_LOGIN_SUCESS)
    {
      error_code = 0;
    }
    else if (login_ret == WEBS_LOGIN_LIMIT)
    {
      error_code = 10103;
    }
    else if(login_ret == WEBS_LOGIN_OTHER)
    {
      error_code = 10102;
    }
    else
    {	
      error_code = 10101;
    }
  }
	else
	{
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Get login params fail.\n");
    error_code = -32700;
	}
    if(error_code == 0)
    {
        ASPE_getPeerInfo(wp,ip_str,&browser_type);
        f = fopen(WLAN_FILE_LOGIN_IP, "w+");
        if(NULL != f)
        {
            fprintf(f, "%s\n", ip_str);
            fclose(f);
        }
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login in %s\n",ip_str);
    }
    else
    {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login wrong password!\n");
    }
    
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "error_code=%d \n", error_code);
  json_webs_get_id(wp, id, MAX_ID_LENGTH);
  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
    json_webs_rsp_result(wp,id, NULL,0);
  
  return;
  // Connie modify start, 2014/9/5
}

void json_webs_set_login_security(webs_t wp, char_t *path, char_t *query)
{
  // Connie modify start, 2014/9/5
  char id[MAX_ID_LENGTH] = {0};
  char ip_str[20] = {0};
  int browser_type = 0;
  int session_id = 0;
  int error_code = 0;
  webs_login_state login_ret;
  FILE *f;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "wp->path: %s, path: %s, query:%s\n", wp->path, path, query);
  
  if (json_webs_check_params(wp, json_set_login_params, COUNTOF(json_set_login_params)))
  {
    if(g_webs_data.webs_pw_username_encrypt)
    {
      char*usr_name=jrd_decrypt(json_set_login_params[0].u.str);
      char *password = jrd_decrypt(json_set_login_params[1].u.str);
      strncpy(json_set_login_params[0].u.str, usr_name,MAX_PARAM_LENGTH);
      strncpy(json_set_login_params[1].u.str, password,MAX_PARAM_LENGTH);
      free(usr_name);
      free(password);
    }
    login_ret = webs_set_login(wp, json_set_login_params, TRUE);
    if (login_ret == WEBS_LOGIN_SUCESS)
    {
      srand((unsigned) time(NULL));
      jrd_webs_token = rand() % (100000000-10000000) + 10000000;
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"**************%d \n", jrd_webs_token);
      memset(jrd_webs_token_str, 0, sizeof(jrd_webs_token_str));
      snprintf(jrd_webs_token_str,sizeof(jrd_webs_token_str)-1,"%d",jrd_webs_token);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"==============%s \n", jrd_webs_token_str);
      json_ui_token_response_params[0].u.val = jrd_webs_token;
      error_code = 0;
    }else if(login_ret == WEBS_LOGIN_ALREADY_LOGIN)
    {
      json_ui_token_response_params[0].u.val = jrd_webs_token;
      error_code = 0;
    }
    else if (login_ret == WEBS_LOGIN_LIMIT)
    {
      error_code = 10103;
    }
    else if(login_ret == WEBS_LOGIN_OTHER)
    {
      error_code = 10102;
    }
    else
    {	
      error_code = 10101;
    }
  }
	else
	{
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Get login params fail.\n");
    error_code = -32700;
	}
    if(error_code == 0)
    {
        ASPE_getPeerInfo(wp,ip_str,&browser_type);
        f = fopen(WLAN_FILE_LOGIN_IP, "w+");
        if(NULL != f)
        {
            fprintf(f, "%s\n", ip_str);
            fclose(f);
        }
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login in %s\n",ip_str);
    }
    else
    {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "login wrong password!\n");
    }

send_respons:
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "error_code=%d \n", error_code);
  json_webs_get_id(wp, id, MAX_ID_LENGTH);
  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
    json_webs_rsp_result(wp,id, json_ui_token_response_params,1);
  
  return;
  // Connie modify start, 2014/9/5
}


//add by PiFangsi 2015-06-18 [add force login api] start
void json_webs_force_login(webs_t wp, char_t *path, char_t *query)
{
  // Connie modify start, 2014/9/5
  char id[MAX_ID_LENGTH] = {0};
  char ip_str[20] = {0};
  int browser_type = 0;
  int session_id = 0;
  int error_code = 0;
  webs_login_state login_ret;
  FILE *f;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "wp->path: %s, path: %s, query:%s\n", wp->path, path, query);

  if (json_webs_check_params(wp, json_set_login_params, COUNTOF(json_set_login_params)))
  {
    login_ret = webs_set_login(wp, json_set_login_params, TRUE);
    if (login_ret == WEBS_LOGIN_SUCESS)
    {
      error_code = 0;
    }
    else if (login_ret == WEBS_LOGIN_LIMIT)
    {
      error_code = 10602;
    }
    else
    {	
      error_code = 10601;
    }
  }
	else
	{
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Get force login params fail.\n");
    error_code = -32700;
	}
  if(error_code == 0)
  {
     ASPE_getPeerInfo(wp,ip_str,&browser_type);
     f = fopen(WLAN_FILE_LOGIN_IP, "w+");
     fprintf(f, "%s\n", ip_str);
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "force login in %s\n",ip_str);
     fclose(f);
  }
  else
  {
     JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "force login wrong password!\n");
  }
    
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "error_code=%d \n", error_code);
  json_webs_get_id(wp, id, MAX_ID_LENGTH);
  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
    json_webs_rsp_result(wp,id, NULL,0);
  
  return;
}
//add by PiFangsi 2015-06-18 [add force login api] end

void tn_webs_set_logout(webs_t wp, char_t *path, char_t *query)
{
	int set_res = WEBS_SET_SUCCESS;

       if(g_webs_data.webs_security == 1)
       {
         if(wp->token_flag == 0)
         {
             JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "tn_webs_set_logout^^^^^\n");
             set_res = 255;
             tn_webs_post_set_return_data(wp,set_res);
             return;
         }
       }
       
	ASPE_userLogout(wp);
	set_timer_clean();
  tn_webs_post_set_return_data(wp,set_res);
}

void json_webs_set_logout(webs_t wp, char_t *path, char_t *query)
{
  // Connie modify start, 2014/9/5
  char id[MAX_ID_LENGTH] = {0};
  
  json_webs_get_id(wp, id, MAX_ID_LENGTH);   
  if(ASPE_userLogout(wp))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"current user not log in!\n");
    json_webs_rsp_result(wp,id,NULL,0);
    return;
  }

	set_timer_clean();
  srand((unsigned) time(NULL));
  jrd_webs_token = rand() % (100000000-10000000) + 10000000;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"**************%d \n", jrd_webs_token);
  memset(jrd_webs_token_str, 0, sizeof(jrd_webs_token_str));
  snprintf(jrd_webs_token_str,sizeof(jrd_webs_token_str)-1,"%d",jrd_webs_token);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"==============%s \n", jrd_webs_token_str);

  system("rm /jrd-resource/resource/wlan/login_ip");
  json_webs_rsp_result(wp,id,NULL,0);
  // Connie modify end, 2014/9/5
}

// Connie add start, 2014/12/15
static int webs_set_password(char *oldpasswd, char *newpasswd)
{
  char*path="/jrd-resource/resource/jrdcfg/smbpasswd";
  char cmd[128] = {0};
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "newpasswd=%s,oldpasswd=%s\n", newpasswd, oldpasswd);
	if(NULL != newpasswd && 0 != strlen(newpasswd))
	{
	    if( NULL != oldpasswd && 0 !=strlen(oldpasswd))
	    {
	      if(0 != strcmp(oldpasswd, g_webs_data.password))
	      {
	        return WEBS_SET_PSW_WRONG_PASSWD;
	      }
		 //add by kang.chen for force to change default password
		if(0 == strcmp(g_webs_data.original_password,newpasswd))
		{
		printf("The new password[%s] is the same as default[%s]!",newpasswd,g_webs_data.original_password);
	        return WEBS_SET_PSW_ORIGINAL_PASSWD;
		}
	    }
	    strcpy(g_webs_data.password,newpasswd);
	    if(0 == webs_update_database(E_WEBS_PARAM_PASSWORD))
	    {
          send_userpw_change_ind();
		  snprintf(cmd, sizeof(cmd),  "echo %s > %s;echo %s >> %s", newpasswd, path,newpasswd, path);
		  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "cmd =%s \n", cmd);
		  system(cmd);
	      return WEBS_SET_PSW_SUCCESS;
	    }
	    else
	    {
	      return WEBS_SET_PSW_FAILED;
	    }
	}
	else
	{
	  return WEBS_GET_UIPARA_WRONG;
	}
}

void tn_webs_set_password(webs_t wp, char_t *path, char_t *query)
{
	int set_res = WEBS_SET_SUCCESS;
	int set_psw_res = WEBS_SET_PSW_SUCCESS;
  char *oldpasswd;
  char *newpasswd;
  if(g_webs_data.webs_security == 1)
  {
      if(wp->token_flag == 0)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "tn_webs_set_password^^^^^\n");
        set_res = 255;
	 webstnHeader(wp);
	 websWrite(wp, T("{\"set_psw_res\":%d,\"error\":%d}"), set_psw_res,set_res);    
	 websDone(wp, 200);
        return;
      }
  }
	oldpasswd = websGetVar(wp, T("oldPassword"), T(""));
	newpasswd = websGetVar(wp, T("newPassword"), T(""));
  set_psw_res = webs_set_password(oldpasswd, newpasswd);
  
	webstnHeader(wp);
	websWrite(wp, T("{\"set_psw_res\":%d,\"error\":%d}"), set_psw_res,set_res);    
	websDone(wp, 200);
}
// Connie add end, 2014/12/15

void json_webs_set_password(webs_t wp, char_t *path, char_t *query)
{
  // Connie modify start, 2014/9/5
  char id[MAX_ID_LENGTH] = {0};
  int error_code = 0;
  int rc;
  uint8 *req_buf = NULL;
  uint8 rsp_buf[3072] = {0};
  int	req_buf_len;
  int	rsp_buf_len;

  if (json_webs_check_params(wp, json_set_passwd_params, COUNTOF(json_set_passwd_params)))
  {
    rc = webs_set_password(json_set_passwd_params[1].u.str, json_set_passwd_params[2].u.str);
    if(rc == WEBS_SET_PSW_WRONG_PASSWD)
    {
      error_code = 10402;
    }
    else if (rc == WEBS_SET_PSW_SUCCESS)
    {
     rc = jrd_create_json_req_buf("samba_set_passwd", NULL, &req_buf, &req_buf_len);
	rc = client_send_sync_msg(local_fd,req_buf,req_buf_len,rsp_buf,&rsp_buf_len,5000);
      error_code  = 0;
    }
    else
    {
      error_code  = 10401;
    }
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Get set passwd params fail.\n");
    error_code = -32700;
	}
    
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "error_code=%d \n", error_code);
  json_webs_get_id(wp, id, MAX_ID_LENGTH);
  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
    json_webs_rsp_result(wp,id,NULL,0);
  return;
  // Connie modify end, 2014/9/5
}

void json_webs_change_password(webs_t wp, char_t *path, char_t *query)
{
  // Connie modify start, 2014/9/5
  char id[MAX_ID_LENGTH] = {0};
  int error_code = 0;
  int rc;
   uint8 *req_buf = NULL;
  uint8 rsp_buf[3072] = {0};
  int	req_buf_len;
  int	rsp_buf_len;
  if (json_webs_check_params(wp, json_set_passwd_params, COUNTOF(json_set_passwd_params)))
  {
    if(g_webs_data.webs_pw_username_encrypt)
    {
      char*old_pwd=jrd_decrypt(json_set_passwd_params[1].u.str);
      char *password = jrd_decrypt(json_set_passwd_params[2].u.str);
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH, "old pwd:%s, new password:%s\n",old_pwd,password);
      rc = webs_set_password(old_pwd, password);
      free(old_pwd);
      free(password);
    }
    else
    {
      rc = webs_set_password(json_set_passwd_params[1].u.str, json_set_passwd_params[2].u.str);
    }
    if(rc == WEBS_SET_PSW_WRONG_PASSWD)
    {
      error_code = 10402;
    }
    else if (rc == WEBS_SET_PSW_SUCCESS)
    {
       rc = jrd_create_json_req_buf("samba_set_passwd", NULL, &req_buf, &req_buf_len);
	rc = client_send_sync_msg(local_fd,req_buf,req_buf_len,rsp_buf,&rsp_buf_len,5000);
      error_code  = 0;
    }
    else if(rc == WEBS_SET_PSW_ORIGINAL_PASSWD)
    {
    	      error_code  = 10403;
    }
    else
    {
      error_code  = 10401;
    }
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Get set passwd params fail.\n");
    error_code = -32700;
	}
send_respons: 
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "error_code=%d \n", error_code);
  json_webs_get_id(wp, id, MAX_ID_LENGTH);
  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
    json_webs_rsp_result(wp,id,NULL,0);
  return;
  // Connie modify end, 2014/9/5
}


void json_webs_init_password(webs_t wp, char_t *path, char_t *query)
{
  // Connie modify start, 2014/9/5
  char id[MAX_ID_LENGTH] = {0};
  int error_code = 0;
  int rc;
  char *password = NULL;
  
  if(E_WEBS_UI_PW_NEEDED == g_webs_data.webs_uipwstate)
  {
    error_code=012101;
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Password needed!!!");
    goto send_respons;
  }

  if (json_webs_check_params(wp, json_init_passwd_params, COUNTOF(json_init_passwd_params)))
  {
    password = jrd_decrypt(json_init_passwd_params[0].u.str);
    if(NULL != password && 0 != strlen(password))
  	{
      strcpy(g_webs_data.password,password);
      if(0 == webs_update_database(E_WEBS_PARAM_PASSWORD))
      {
        send_userpw_change_ind();
        g_webs_data.webs_uipwstate = E_WEBS_UI_PW_NEEDED;
        webs_update_database(E_WEBS_PARAM_UIPWSTATE);
        error_code = WEBS_SET_PSW_SUCCESS;
      }
      else
      {
        error_code = 012101;
      }
  	}
  	else
  	{
  	  error_code = 012101;
  	}
  	
    free(password);
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Get set passwd params fail.\n");
    error_code = -32700;
	}

send_respons:

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "error_code=%d \n", error_code);
  json_webs_get_id(wp, id, MAX_ID_LENGTH);
  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
    json_webs_rsp_result(wp,id,NULL,0);
  return;
  // Connie modify end, 2014/9/5
}


void tn_webs_set_login_userpw(webs_t wp, char_t * path, char_t * query)
{
  char* new_psw = NULL;
	char* new_usr= NULL;
	int set_res = WEBS_SET_SUCCESS;
  int rc1,rc2;
  if(g_webs_data.webs_security == 1)
  {
      if(wp->token_flag == 0)
      {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "tn_webs_set_login_userpw^^^^^\n");
        set_res = 255;
        tn_webs_post_set_return_data(wp,set_res);
        return;
      }
  }

	new_psw = websGetVar(wp, T("newPassword"), T(""));
	new_usr = websGetVar(wp, T("newUsername"), T(""));
	
	if((new_psw != NULL)&&(0 != strlen(new_psw))&&(new_usr != NULL)&&(0 != strlen(new_usr)))
  {
    strcpy(g_webs_data.password,new_psw);
    strcpy(g_webs_data.username,new_usr);
    rc1 = webs_update_database(E_WEBS_PARAM_PASSWORD);
    rc2 = webs_update_database(E_WEBS_PARAM_USER_NAME);
    if(0 == rc1 && 0 == rc2)
    {
      send_userpw_change_ind();
      set_res  = WEBS_SET_SUCCESS;

      g_webs_data.usrpwd_changed = 1;
      webs_update_database(E_WEBS_PARAM_USRPWD_CHANGED);
    }
    else
    {
      set_res  = WEBS_SET_FAILED;
    }
	}
	else
	{
	  set_res  = WEBS_GET_UIPARA_WRONG;
	}
	tn_webs_post_set_return_data(wp,set_res);
}
// zengyong add, 2015/3/4
void webs_get_login_remain_info(webs_t wp, char_t * path, char_t * query)
{
  int login_count_remain = g_webs_data.max_retry_times;
  int login_time_remain = 0;

  if (g_login_fail_count > 0)
  {
    login_count_remain = login_count_remain - g_login_fail_count;
  }

  if (login_count_remain == 0)
  {
    login_time_remain = g_webs_data.unlock_time;
    login_time_remain = login_time_remain - g_login_fail_time_count;
  }

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "Login remain info: loginCountRemain:%d; loginTimeRemain:%d \n", login_count_remain, login_time_remain);
  webstnHeader(wp);
  websWrite(wp, T("{\"loginCountRemain\":%d,\"loginTimeRemain\":%d,\"error\":%d}"),login_count_remain,login_time_remain,WEBS_SET_SUCCESS);
  websDone(wp, 200);
}

int random_number = 0;
void tn_webs_get_Token(webs_t wp, char_t * path, char_t * query)
{
  random_number = rand();
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "tn_webs_get_Token:%d \n",random_number);
  webstnHeader(wp);
  websWrite(wp, T("{\"token\":%d,\"error\":%d}"),random_number,WEBS_SET_SUCCESS);
  websDone(wp, 200);
}

void json_webs_get_Token(webs_t wp, char_t * path, char_t * query)
{
  char id[MAX_ID_LENGTH] = {0};
  int token = 0;
  char ip_str[20] = {0};
  int session_id = 0;
  
  if(E_WEBS_UI_PW_NOT_NEEDED == g_webs_data.webs_uipwstate)
  {
    token = jrd_webs_token;
    goto send_respons;
  }
  
  if(hasUserLogined(ip_str, &session_id) == TRUE)
  {
    char ip_str2[20] = {0};
    int browser_type = 0;
    memset(ip_str2,0,sizeof(ip_str2));

    ASPE_getPeerInfo(wp,ip_str2,&browser_type);

    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "ip_str:%s, ip_str2:%s, browser_type:%d\n",ip_str, ip_str2, browser_type);
    if(strcmp(ip_str,ip_str2) == 0)
    {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "Already log in \n");
      token = jrd_webs_token;
    }
  }
  /*srand((unsigned) time(NULL));
  jrd_webs_token = rand() % (100000000-10000000) + 10000000;
  printf("**************%d \n", jrd_webs_token);*/
send_respons:
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW, "json_webs_get_Token:%d \n",jrd_webs_token);
  json_ui_token_response_params[0].u.val = token;
  json_webs_get_id(wp, id, MAX_ID_LENGTH);   
  json_webs_rsp_result(wp, id, json_ui_token_response_params,1);
}




// jie.li add start, 2014/9/5
void json_webs_heart_beat(webs_t wp, char_t * path, char_t * query)
{
  char id[MAX_ID_LENGTH] = {0};
  //fengzhou.zhao add start 2014/11/08
  if(1 == EGOSM_isUserLogined(wp))
  {  
    EGOSM_resetSessionHeartbeat(wp);
  }
  //fengzhou.zhao add end 2014/11/08
  json_webs_get_id(wp, id, MAX_ID_LENGTH);   
  json_webs_rsp_result(wp,id,NULL,0);
}
// jie.li add end, 2014/9/5

static void tn_webs_save_dmesg(webs_t wp, char_t * path, char_t * query)
{
	char *store_path;
	char cmd_buf[256] = {0};
  int set_res = WEBS_SET_SUCCESS;

	store_path = websGetVar(wp, T("store_path"), T(""));

	if((store_path == NULL)||(0 == strlen(store_path)))
	{
	  store_path = "/cache/kernel_msg.log";
	}
	snprintf(cmd_buf,sizeof(cmd_buf),"dmesg > %s",store_path);
	set_res = system(cmd_buf);
	
	memset(cmd_buf,0,sizeof(cmd_buf));
	snprintf(cmd_buf,sizeof(cmd_buf),"ln -s %s /jrd-resource/resource/webrc/www/kernel_msg.log",store_path);
	system(cmd_buf);

	tn_webs_post_set_return_data(wp,set_res);
}

static void tn_webs_start_catch_kmsg(webs_t wp, char_t * path, char_t * query)
{
	char *store_path;
	char cmd_buf[256] = {0};
  int set_res = WEBS_SET_SUCCESS;

	store_path = websGetVar(wp, T("store_path"), T(""));

	if((store_path == NULL)||(0 == strlen(store_path)))
	{
	  store_path = "/cache/dynamic_kernel_msg.log";
	}
	snprintf(cmd_buf,sizeof(cmd_buf),"cat /proc/kmsg > %s &",store_path);
	set_res = system(cmd_buf);
	
	memset(cmd_buf,0,sizeof(cmd_buf));
	snprintf(cmd_buf,sizeof(cmd_buf),"ln -s %s /jrd-resource/resource/webrc/www/dynamic_kernel_msg.log",store_path);
	system(cmd_buf);

	tn_webs_post_set_return_data(wp,set_res);
}

void tn_webs_get_password_save_info(webs_t wp, char_t *path, char_t *query)
{
  webstnHeader(wp);
  websWrite(wp, T("{\"username\":\"%s\",\"password\":\"%s\",\"save_flag\":%d,\"error\":%d}"),g_webs_data.username,g_webs_data.password,g_webs_data.remember_usrpwd,WEBS_SET_SUCCESS);
  websDone(wp, 200);
}

void tn_webs_set_password_save_info(webs_t wp, char_t * path, char_t * query)
{
  char* username = NULL;
	char* password= NULL;
	char* save_flag= NULL;
	int set_res = WEBS_SET_SUCCESS;
  int rc;

	username  = websGetVar(wp, T("username"), T(""));
	password  = websGetVar(wp, T("password"), T(""));
	save_flag = websGetVar(wp, T("save_flag"), T(""));
	
	if((save_flag != NULL)&&(0 != strlen(save_flag)))
  {
    g_webs_data.remember_usrpwd = atoi(save_flag);
    rc = webs_update_database(E_WEBS_PARAM_REMEMBER_USRPWD);
    if(0 == rc)
    {
      set_res  = WEBS_SET_SUCCESS;
    }
    else
    {
      set_res  = WEBS_SET_FAILED;
    }
	}
	else
	{
	  set_res  = WEBS_GET_UIPARA_WRONG;
	}
	tn_webs_post_set_return_data(wp,set_res);
}

void tn_webs_get_password_change_flag(webs_t wp, char_t *path, char_t *query)
{
  webstnHeader(wp);
  websWrite(wp, T("{\"change_flag\":%d,\"error\":%d}"),g_webs_data.usrpwd_changed,WEBS_SET_SUCCESS);
  websDone(wp, 200);
}


static struct {
  char str[32];
  int dig;
} str_log_level[] = { {"LOW",4},  {"MEDIAM",8},  {"HIGH",8},  {"ERROR",16} };

int jrd_oem_log_level = JRD_OEM_LOG_LOW;
void json_webs_set_log_level(webs_t wp, char_t * path, char_t * query)
{
  char_t	*loglevel = NULL;
  int set_res = WEBS_SET_SUCCESS;
  int json_set_log_level = 0;
  int i = 0;

  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "call json_webs_set_log_level\n");
  if (json_webs_check_params(wp, json_set_log_level_params, COUNTOF(json_set_log_level_params)))
  {
    for(i = 0; i < COUNTOF(str_log_level); i++)
    {
       if(NULL != strstr(json_set_log_level_params[0].u.str,str_log_level[i].str))
       {
          json_set_log_level = str_log_level[i].dig;
          break;
       }
    }

    switch(json_set_log_level)
    {
    case JRD_OEM_LOG_LOW:
    //case JRD_OEM_LOG_MEDIAM:
    case JRD_OEM_LOG_HIGH:
    case JRD_OEM_LOG_ERROR:
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_set_log_level=%d\n",json_set_log_level);
      jrd_oem_log_level = json_set_log_level;
      break;
    default:
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "invalid params\n");
      set_res = WEBS_GET_UIPARA_WRONG;
      break;
    }
  }
  else
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "invalid params\n");
	  set_res = WEBS_GET_UIPARA_WRONG;
  }

  webstnHeader(wp);
  websWrite(wp, T("{\"error\":%d}"),set_res);
  websDone(wp, 200);
}
  /////////////////////add begin by wei.huang.sz/////////////////
void json_webs_get_wifi_UIPwState(webs_t wp, char_t *path, char_t *query)
{
  char id[MAX_ID_LENGTH] = {0};
  int error_code = 0;
  FILE *f;
  char UIPwState[2] = {0};

  json_ui_wifi_pwstate_response_params[0].u.val = g_webs_data.wifi_password_changed;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_webs_get_wifi_UIPwState %d\n",json_ui_wifi_pwstate_response_params[0].u.val);
  
  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
  json_webs_rsp_result(wp, id, json_ui_wifi_pwstate_response_params,1);
  return;
}

void json_webs_set_wifi_UIPwState(webs_t wp, char_t *path, char_t *query)
{
  char id[MAX_ID_LENGTH] = {0};
  int error_code = 0;
  FILE *f;
  //char UIPwState = '0';
  
  if (json_webs_check_params(wp, json_ui_wifi_pwstate_params, COUNTOF(json_ui_wifi_pwstate_params)))
  {

         JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_webs_Set_UIPwState %d\n",json_ui_wifi_pwstate_params[0].u.val);
         g_webs_data.wifi_password_changed =  json_ui_wifi_pwstate_params[0].u.val;
         webs_update_database(E_WEBS_PARAM_WIFI_PASSWORD_CHANGED);
  }
  else
  {
      error_code = 11201;
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "error_code=%d \n", error_code);
  }
  
send_respons:

  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
    json_webs_rsp_result(wp,id, NULL,0);
  return;
}

void json_webs_get_password_change_flag(webs_t wp, char_t *path, char_t *query)
{
  char id[MAX_ID_LENGTH] = {0};
  int error_code = 0;
  FILE *f;
  char UIPwState[2] = {0};
//add by ck for password change flag

  if(0 == strcmp(g_webs_data.original_password, g_webs_data.password))
  	g_webs_data.usrpwd_changed = 0;//not changed
  else
  	g_webs_data.usrpwd_changed = 1; //already changed
      webs_update_database(E_WEBS_PARAM_USRPWD_CHANGED);

  json_ui_pwstate_change_flag_response_params[0].u.val = g_webs_data.usrpwd_changed;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_webs_get_wifi_UIPwState %d\n",json_ui_pwstate_change_flag_response_params[0].u.val);
  
  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
  json_webs_rsp_result(wp, id, json_ui_pwstate_change_flag_response_params,1);
  return;
}

void json_webs_set_password_change_flag(webs_t wp, char_t *path, char_t *query)
{
  char id[MAX_ID_LENGTH] = {0};
  int error_code = 0;
  FILE *f;
  //char UIPwState = '0';
  
  if (json_webs_check_params(wp, json_ui_pwstate_change_flag_params, COUNTOF(json_ui_pwstate_change_flag_params)))
  {

         JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "json_webs_Set_UIPwState %d\n",json_ui_pwstate_change_flag_params[0].u.val);
         g_webs_data.usrpwd_changed =  json_ui_pwstate_change_flag_params[0].u.val;
         webs_update_database(E_WEBS_PARAM_USRPWD_CHANGED);
  }
  else
  {
      error_code = 11201;
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "error_code=%d \n", error_code);
  }
  
send_respons:

  if (error_code)
    json_webs_rsp_err(wp,error_code,id);
  else
    json_webs_rsp_result(wp,id, NULL,0);
  return;
}

void json_webs_parser_err_handler(webs_t wp, char_t *path, char_t *query)
{
  char id[MAX_ID_LENGTH] = {0};
  int error_code = -32700;
  FILE *f;
  //char UIPwState = '0';  
  json_webs_rsp_err(wp,error_code,id);
  return;
}

  /////////////////////add end by wei.huang.sz/////////////////
/*******************************************************************/
/**
 *asp function  regist
 */
static webs_get_req_type webs_get_req_tbl[] = {
 {"webs_get_server_info",webs_get_server_info},
};

/**
 *asp function  handle  form post
 */
static webs_post_req_type webs_post_req_tbl[] = {
  // jie.li modify start, 2014/9/5
  {"GetLoginState",   json_webs_get_login_state},         // For json web
  {"Login",           json_webs_set_login_security},//json_webs_set_login},               // For json web
  //add by PiFangsi 2015-06-18 [add force login api] start
  {"ForceLogin",      json_webs_force_login},             // For json web
  //add by PiFangsi 2015-06-18 [add force login api] end
  {"Logout",          json_webs_set_logout},              // For json web
  {"ChangePassword",  json_webs_change_password},//json_webs_set_password},            // For json web
  {"HeartBeat",       json_webs_heart_beat},              // For json web
  // jie.li modify end, 2014/9/5
  {"getLoginState",      tn_webs_get_login_state},         // For tn web
  {"setLogin",           tn_webs_set_login},               // For tn web
  {"setLogout",          tn_webs_set_logout},              // For tn web
  {"setPassword",        tn_webs_set_password},            // For tn web
  {"setUsernameAndPW",   tn_webs_set_login_userpw},        // For tn web
  {"getPasswordSaveInfo",           tn_webs_get_password_save_info},
  {"setPasswordSaveInfo",           tn_webs_set_password_save_info},
  {"getPasswordChangeWarningFlag",  tn_webs_get_password_change_flag},

  {"getLoginRemainInfo", webs_get_login_remain_info},
  {"saveDmesg",          tn_webs_save_dmesg},        // For tn web
  {"startCatchKmsg",     tn_webs_start_catch_kmsg},        // For tn web
  {"getToken",   tn_webs_get_Token},        // For ee customer security 
  {"setWebsLog",   json_webs_set_log_level},        //For setting log level on tn web
  /////////////////////add begin by wei.huang.sz/////////////////
  //{"GetPasswordSaveInfo",           json_webs_get_PasswordSaveInfo},
  //{"SetPasswordSaveInfo",           json_webs_set_PasswordSaveInfo},
  {"GetUIPwState",            json_webs_get_UIPwState},
  //{"SetUIPwState",            json_webs_Set_UIPwState}, delete by fengzhou for security issue
  //{"GetUserNameAndPw",        json_webs_get_UserNameAndPw},

  {"InitPwSecurity",          json_webs_init_password},
  {"LoginSecurity",           json_webs_set_login_security},
  {"ChangePwSecurity",        json_webs_change_password},
  {"GetwifiPasswordChangeFlag",  json_webs_get_wifi_UIPwState},
  {"SetwifiPasswordChangeFlag",  json_webs_set_wifi_UIPwState},
  {"GetPasswordChangeFlag",      json_webs_get_password_change_flag},
  {"SetPasswordChangeFlag",      json_webs_set_password_change_flag},
  {"saveDmesg",                  json_webs_parser_err_handler},//deactive the api here for security, so no need modify MDM9240 core_app and parser file
  {"startCatchKmsg",             json_webs_parser_err_handler},//deactive the api here for security, so no need modify MDM9240 core_app and parser file
  /////////////////////add end by wei.huang.sz/////////////////
  /* temp add here for early debug*/
 // {"getWlanInfo",        tn_webs_get_wlan_info},         
  //{"getWanInfo",         tn_webs_get_wan_info},         
};

/**********************************************************************/
//init register function

void webs_asp_init(void)
{
  int i;
  
  srand((unsigned) time(NULL));
  jrd_webs_token = rand() % (100000000-10000000) + 10000000;
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"**************%d \n", jrd_webs_token);
  memset(jrd_webs_token_str, 0, sizeof(jrd_webs_token_str));
  snprintf(jrd_webs_token_str,sizeof(jrd_webs_token_str)-1,"%d",jrd_webs_token);
  JRD_OEM_LOG_INFO(JRD_OEM_LOG_HIGH,"==============%s \n", jrd_webs_token_str);

  for (i = 0; i < COUNTOF(webs_get_req_tbl); i ++) {
    websAspDefine(webs_get_req_tbl[i].func_name, webs_get_req_tbl[i].func_ptr);
  }

  for (i = 0; i < COUNTOF(webs_post_req_tbl); i ++) {
    websFormDefine(webs_post_req_tbl[i].func_name, webs_post_req_tbl[i].func_ptr);
  }
  if (0 != websLocalappApiinit()){
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"==============websLocalappApiinit ERROR \n");
  }
}


