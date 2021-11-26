#ifndef _SOCK_CLT_H_
#define _SOCK_CLT_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "queue.h"
#include "wsIntrn.h"
#include <common/jrd_common_def.h>
#include <json-c/json.h>

#define JRD_SOCK_WEB_TX_BUF_SIZE 8192
#define JRD_SOCK_WEB_RX_BUF_SIZE 3072
#define JRD_OEM_SOCK_CLT_STACK_SIZE 128*1024

typedef enum jrd_params_id {
	JRD_PARAMS_ID_INVALID = -1,
	JRD_PARAMS_ID_MIN = 0,
	JRD_PARAMS_ID_MAC_ADDRESS = JRD_PARAMS_ID_MIN,      //MacAddress
	JRD_PARAMS_ID_IP_ADDRESS,                           //IpAddress
	JRD_PARAMS_ID_LANGUAGE,                             //Language
	JRD_PARAMS_ID_IMEI,                                 //IMEI
	JRD_PARAMS_ID_SOFTWARE_VER,                         //SwVersion
	JRD_PARAMS_ID_HARDWARE_VER,                         //HwVersion
	JRD_PARAMS_ID_DEVICE_NAME,                          //DeviceName
	JRD_PARAMS_ID_NETWORK_NAME,                         //NetworkName
	JRD_PARAMS_ID_NETWORK_TYPE,                         //NetworkType
	JRD_PARAMS_ID_ROAMING,                              //Roaming
	JRD_PARAMS_ID_ROAMING_CONNECT,                              //Roaming
	JRD_PARAMS_ID_SIGNAL_STRENGTH,                      //SignalStrength
	JRD_PARAMS_ID_CONNECTION_STATUS,                    //ConnectionStatus
	JRD_PARAMS_ID_CONNECTION_TIME,                      //ConnectionTime
	JRD_PARAMS_ID_DATA_USAGE_CURRENT,                   //CurrentDataUsage
	JRD_PARAMS_ID_DATA_USAGE_TOTAL,                    //TotalDataUsage
	JRD_PARAMS_ID_WLAN_STATE,                           //WlanState
	JRD_PARAMS_ID_TOTAL_CONN_NUM,                       //TotalConnNum
	JRD_PARAMS_ID_SMS_STATE,                            //SmsState
	JRD_PARAMS_ID_BAT_CAP,                              //bat_cap
	JRD_PARAMS_ID_BAT_LEVEL,                            //bat_level
	JRD_PARAMS_ID_CHG_STATE,                            //chg_state
	JRD_PARAMS_ID_REVCHG_ENABLED,                       //revchg_enable
	JRD_PARAMS_ID_SIM_STATE,
	JRD_PARAMS_ID_PIN_STATE,
	JRD_PARAMS_ID_PIN_REMAINING_TIMES,
	JRD_PARAMS_ID_PUK_REMAINING_TIMES,
	JRD_PARAMS_ID_SIMLOCK_STATE,
	JRD_PARAMS_ID_SOMLOCK_REMAINING_TIMES,
  JRD_PARAMS_ID_WLAN_AP_MODE,
  JRD_PARAMS_ID_WMODE,
  JRD_PARAMS_ID_SSID,
  JRD_PARAMS_ID_SSID_HIDDEN,
  JRD_PARAMS_ID_CHANNEL,
  JRD_PARAMS_ID_MAX_NUMSTA,
  JRD_PARAMS_ID_WLAN_CLIENT_NUM,
  JRD_PARAMS_ID_SECURITY_MODE,
  JRD_PARAMS_ID_WEP_TYPE,
  JRD_PARAMS_ID_WEP_KEY,
  JRD_PARAMS_ID_WPA_TYPE,
  JRD_PARAMS_ID_WPA_KEY,
  JRD_PARAMS_ID_COUNTRY_CODE,
  JRD_PARAMS_ID_AP_ISOLATION,
  JRD_PARAMS_ID_WMODE_5G,
  JRD_PARAMS_ID_SSID_5G,
  JRD_PARAMS_ID_SSID_HIDDEN_5G,
  JRD_PARAMS_ID_CHANNEL_5G,
  JRD_PARAMS_ID_MAX_NUMSTA_5G,
  JRD_PARAMS_ID_SECURITY_MODE_5G,
  JRD_PARAMS_ID_WEP_TYPE_5G,
  JRD_PARAMS_ID_WEP_KEY_5G,
  JRD_PARAMS_ID_WPA_TYPE_5G,
  JRD_PARAMS_ID_WPA_KEY_5G,
  JRD_PARAMS_ID_COUNTRY_CODE_5G,
  JRD_PARAMS_ID_AP_ISOLATION_5G,
  JRD_PARAMS_ID_FOTA_STATE,
  JRD_PARAMS_ID_FOTA_VERSION,
  JRD_PARAMS_ID_FOTA_TOTAL_SIZE,
  JRD_PARAMS_ID_FOTA_DL_SIZE,
  JRD_PARAMS_ID_FOTA_DL_STATE,
  JRD_PARAMS_ID_FOTA_DL_SPEED,
  JRD_PARAMS_ID_SMS_MAX_COUNT,
  JRD_PARAMS_ID_SMS_USE_COUNT,
  JRD_PARAMS_ID_SMS_UNREAD_COUNT,
  JRD_PARAMS_ID_SMS_LEFT_COUNT,
  JRD_PARAMS_ID_SMS_UNREAD_REPORT,
  JRD_PARAMS_ID_SMS_ID,
  JRD_PARAMS_ID_SMS_KEY,
  JRD_PARAMS_ID_SMS_PAGE_NUM,
  JRD_PARAMS_ID_NW_REG_STATUS,
  JRD_PARAMS_ID_WLAN_SSID,
  JRD_PARAMS_ID_WLAN_KEY,
  JRD_PARAMS_ID_USAGE_TOTAL_CONN_TIMES,
  JRD_PARAMS_ID_USAGE_CURR_CONN_TIMES,
  JRD_PARAMS_ID_USAGE_MONTHLY_HOME_UL,
  JRD_PARAMS_ID_USAGE_MONTHLY_HOME_DL,
  JRD_PARAMS_ID_USAGE_MONTHLY_HOME_UDL,
  JRD_PARAMS_ID_USAGE_MONTHLY_ROAM_UL,
  JRD_PARAMS_ID_USAGE_MONTHLY_ROAM_DL,
  JRD_PARAMS_ID_USAGE_MONTHLY_ROAM_UDL,
  JRD_PARAMS_ID_USAGE_MONTHLY_PLAN,
  JRD_PARAMS_ID_USAGE_UNIT_VALUE,
  JRD_PARAMS_ID_USAGE_LAST_USAGE,
  
  JRD_PARAMS_ID_TN_WLAN_STATE,   
  JRD_PARAMS_ID_TN_WLAN_AP_MODE,    
  JRD_PARAMS_ID_TN_WLAN_SSID_STATE, 
  JRD_PARAMS_ID_TN_WLAN_WMODE,      
  JRD_PARAMS_ID_TN_WLAN_SSID,      
  JRD_PARAMS_ID_TN_WLAN_HIDDEN_SSID,
  JRD_PARAMS_ID_TN_WLAN_CHANNEL,    
  JRD_PARAMS_ID_TN_WLAN_MAX_NUMSTA, 
  JRD_PARAMS_ID_TN_WLAN_CURR_NUM,   
  JRD_PARAMS_ID_TN_WLAN_SECURITY_MODE,
  JRD_PARAMS_ID_TN_WLAN_WEP_SEC,      
  JRD_PARAMS_ID_TN_WLAN_WEP_KEY,      
  JRD_PARAMS_ID_TN_WLAN_WPA_SEC,      
  JRD_PARAMS_ID_TN_WLAN_WPA_PASSPHRASE,  
  JRD_PARAMS_ID_TN_WLAN_WIFI_COUNTY_CODE,
  JRD_PARAMS_ID_TN_WLAN_AP_STATUS,      
  JRD_PARAMS_ID_TN_WLAN_WMODE_5G,     
  JRD_PARAMS_ID_TN_WLAN_SSID_5G,     
  JRD_PARAMS_ID_TN_WLAN_HIDDEN_SSID_5G,
  JRD_PARAMS_ID_TN_WLAN_CHANNEL_5G,    
  JRD_PARAMS_ID_TN_WLAN_MAX_NUMSTA_5G, 
  JRD_PARAMS_ID_TN_WLAN_SSID_2_STATE,  
  JRD_PARAMS_ID_TN_WLAN_SSID_2,     
  JRD_PARAMS_ID_TN_WLAN_HIDDEN_SSID_2, 
  JRD_PARAMS_ID_TN_WLAN_MAX_NUMSTA_2,  
  JRD_PARAMS_ID_TN_WLAN_CURR_NUM_2,    
  JRD_PARAMS_ID_TN_WLAN_SECURITY_MODE_2,
  JRD_PARAMS_ID_TN_WLAN_WEP_SEC_2,     
  JRD_PARAMS_ID_TN_WLAN_WEP_KEY_2,     
  JRD_PARAMS_ID_TN_WLAN_WPA_SEC_2,     
  JRD_PARAMS_ID_TN_WLAN_WPA_PASSPHRASE_2,
  JRD_PARAMS_ID_TN_WLAN_AP_STATUS_2,     
  JRD_PARAMS_ID_TN_WLAN_SSID_5G_2_STATE, 
  JRD_PARAMS_ID_TN_WLAN_SSID_5G_2,      
  JRD_PARAMS_ID_TN_WLAN_HIDDEN_SSID_5G_2,   
  JRD_PARAMS_ID_TN_WLAN_MAX_NUMSTA_5G_2, 
  JRD_PARAMS_ID_TN_WLAN_SECURITY_MODE_5G_2,
  JRD_PARAMS_ID_TN_WLAN_WEP_SEC_5G_2,      
  JRD_PARAMS_ID_TN_WLAN_WEP_KEY_5G_2,      
  JRD_PARAMS_ID_TN_WLAN_WPA_SEC_5G_2,      
  JRD_PARAMS_ID_TN_WLAN_WPA_PASSPHRASE_5G_2,
  JRD_PARAMS_ID_TN_WLAN_AP_STATUS_5G_2,
  
	JRD_PARAMS_ID_MAX,
	E_PARAMS_ID_MAX,	
} jrd_params_id_t;


typedef struct jrd_sock_info_s
{
  int sock_fd;
  uint8 tx_buf[JRD_SOCK_WEB_TX_BUF_SIZE];
  uint8 rx_buf[JRD_SOCK_WEB_RX_BUF_SIZE];
}jrd_sock_info_t;

typedef struct rd_sock_msg_s
{
  jrd_sock_hdr_t      msg_hdr;
  uint32              data_len;
  uint8*              data_buf;
}jrd_sock_msg_t;

typedef struct jrd_sock_msg_q_s
{
  q_link_type                 link;
  jrd_sock_msg_t* sock_msg;
  int sock_fd;
}jrd_sock_msg_q_t;

#define MAX_PARAM_LEN        128

typedef struct {
  jrd_params_id_t param_id;
  char           *name;
  boolean         got_value;
  json_type       type;
  
  union {
    char      str[MAX_PARAM_LEN];  // used for UNICODE
    uint64_t       val_int;
  } u;
} jrd_param_info_type;

extern int local_fd;
extern int  start_sock_client(char *path);
extern int websJRDJsonHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	char_t *url, char_t *path, char_t *query);
// Connie add start, 2014/12/16
extern int websJRDtnHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	char_t *url, char_t *path, char_t *query);
extern int websJRDXMLHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
	char_t *url, char_t *path, char_t *query);
// Connie add end, 2014/12/16
extern void send_userpw_change_ind(void);

#ifdef __cplusplus
}
#endif

#endif


