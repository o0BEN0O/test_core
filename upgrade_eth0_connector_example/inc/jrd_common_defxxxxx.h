#ifndef _JRD_COMMON_DEF_H_
#define _JRD_COMMON_DEF_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PACK      __attribute__((packed))

typedef char boolean;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

#define PACK_HEAD 0x7061636B
#define JRD_SOCK_INVALID_FD (-1)
#define JRD_SERVER_PATH "/dev/socket/qmux_webs/server_webs"

typedef enum
{
   E_MSG_TYPE_MIN = 0,
   E_JSON_MSG_REQ = E_MSG_TYPE_MIN,
   E_JSON_MSG_RSP,
   E_MSG_IND,
   E_TN_MSG_REQ, // Connie add, 2014/11/15
   E_TN_MSG_RSP, // Connie add, 2014/11/15
   E_VODAFONE_REQ,
   E_VODAFONE_RSP,
   E_MSG_TYPE_MAX,
}e_jrd_sock_msg_type;

typedef enum
{
   E_CUST_DEFAULT = 0,
   E_CUST_EE_DEVINFO,
   E_CUST_JSON_RESTORE
  }e_jrd_cust_flag_type;

typedef struct
{
  e_jrd_sock_msg_type  msg_type;
  e_jrd_cust_flag_type cust_flag;
  int                  error_code;
  uint32               usr_p;
  uint32               content_len; // Connie add, 2014/9/8
  uint8                data_start;
  uint8                data_complete;
}jrd_sock_hdr_t;

typedef enum
{
   E_MSG_IND_TYPE_MIN = 0,
   E_MSG_USERPW_CHANGE_IND = E_MSG_IND_TYPE_MIN,
   E_MSG_IND_TYPE_MAX,
}e_jrd_sock_msg_ind_type;

typedef enum
{
   MSG_SYNC=0x0,
   MSG_ASYNC=0x1,
   MSG_IND=0x2,
   MSG_MAX=0x3
}msg_enum_type;

typedef  struct
{
  uint32  txn_id;
  msg_enum_type msg_type;
  boolean msg_not_finish;
} PACK msg_head_t;

typedef struct sock_pack_head
{
  uint32  pack_head;
  uint32  pack_len;
}pack_head_t;

typedef enum
{
  IND_TO_CLT_MIN_ID = 0,
  IND_TO_CLT_CONN_STATE,
  IND_TO_CLT_NEW_SMS,
  IND_TO_CLT_SIG_STRENGTH,
  IND_TO_CLT_NETWORK_INFO,
  IND_TO_CLT_BATTERY_INFO,
  IND_TO_CLT_USIM_INFO,
  IND_TO_CLT_FOTA_INFO,
  IND_TO_CLT_WIFI_STATE_INFO,
  IND_TO_CLT_WIFI_CLIENT_NUM_INFO,
  IND_TO_CLT_WIFI_SETTING_INFO,
  IND_TO_CLT_SMS_INFO,
  IND_TO_CLT_SCREEN_TURN_ON,
  IND_TO_CLT_POWER_OFF,
  IND_TO_CLT_SHOW_POWER_OFF_LOGO,
  IND_TO_CLT_LANGUAGE_CHANGE,
  IND_TO_CLT_SCREEN_FLUSH,
  IND_TO_CLT_SET_LOG_LEVEL,
  IND_TO_CLT_SET_DIAG_MODE,
  IND_TO_CLT_WIFI_WPS_START,
  IND_TO_CLT_WEBS_REDIR,
  IND_TO_CLT_MAX_ID,
}e_ind_to_clt_id_type;
#endif


