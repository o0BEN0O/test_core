#ifndef HEAD_H
#define HEAD_H

#include <stdio.h>

#define ACTIVE 0
#define TRANS  1
#define FAULT 2

#define power_led_normal 1
#define power_led_upgrade 2


#define LED_NMEA_BIT_MASK	(0x3 << 2)|(0x3 << 6)
#define LED_NMEA_RED_ON		(0x0 << 2)|(0x0 << 6)|(0x1 << 3)|(0x1 << 7)
#define LED_NMEA_GREEN_ON	(0x0 << 3)|(0x0 << 7)|(0x1 << 2)|(0x1 << 6)
#define LED_NMEA_ORANGE_ON	(0x0 << 2)|(0x0 << 6)|(0x0 << 3)|(0x0 << 7)
#define LED_NMEA_OFF		(0x3 << 2)|(0x3 << 6)

#define LED_WIFI2_BIT_MASK	(0x3 << 4)
#define LED_WIFI2_RED_ON	(0x1 << 4)|(0x0 << 5)/*those 2 pins is exchange,hareware designed*/
#define LED_WIFI2_GREEN_ON	(0x1 << 5)|(0x0 << 4)/*those 2 pins is exchange,hareware designed*/
#define LED_WIFI2_ORANGE_ON (0x0 << 4)|(0x0 << 5)
#define LED_WIFI2_OFF 		(0x3 << 4)

#define LED_CLOUD_BIT_MASK	(0x3 << 10)
#define LED_CLOUD_RED_ON	(0x0 << 10)|(0x1 << 11)
#define LED_CLOUD_GREEN_ON	(0x0 << 11)|(0x1 << 10)
#define LED_CLOUD_ORANGE_ON	(0x0 << 11)|(0x0 << 10)
#define LED_CLOUD_OFF		(0x3 << 10)

#define LED_WIFI1_BIT_MASK  (0x3 << 14)
#define LED_WIFI1_RED_ON	(0x0 << 14)|(0x1 << 15)
#define LED_WIFI1_GREEN_ON	(0x0 << 15)|(0x1 << 14)
#define LED_WIFI1_ORANGE_ON	(0x0 << 15)|(0x0 << 14)
#define LED_WIFI1_OFF 		(0x3 << 14)

#define LED_LTE_BIT_MASK 	(0x3 << 12)
#define LED_LTE_3G_ON		(0x0 << 12)|(0x0 << 13)/*ORANGE*/
#define LED_LTE_4G_ON		(0x1 << 12)|(0x0 << 13)/*GREEN*/
#define LED_LTE_NO_SIG		(0x0 << 12)|(0x1 << 13)/*RED*/
#define LED_LTE_OFF			(0x3 << 12)


typedef enum 
{
	E_SIMNotInserted = 0,
	E_SIMReady,
	E_NoNetwork,
	E_2GNetwork,
	E_3GNetwork,
	E_4GNetwork,
}en4GModuleType;

extern void init_led_control(void);
extern void led_control(int iLedStatus,int iLedBitMask);

extern int QuectelATProcess(void);
extern void Output(char *str);

void jrd_oem_NETWORK_module_init(void);

#endif
