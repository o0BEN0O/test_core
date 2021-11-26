#ifndef HEAD_H
#define HEAD_H

#include <stdio.h>

#define LED_WIFI1_RED_BIT_MASK		(0x1 << 13)
#define LED_WIFI1_GREEN_BIT_MASK	(0x1 << 12)
#define LED_WIFI2_RED_BIT_MASK		(0x1 << 11)
#define LED_WIFI2_GREEN_BIT_MASK	(0x1 << 10)
#define LED_LAN1_RED_BIT_MASK		(0x1 << 7)
#define LED_LAN1_GREEN_BIT_MASK		(0x1 << 6)
#define LED_LAN2_RED_BIT_MASK		(0x1 << 5)
#define LED_LAN2_GREEN_BIT_MASK		(0x1 << 4)
#define LED_LAN3_RED_BIT_MASK		(0x1 << 3)
#define LED_LAN3_GREEN_BIT_MASK		(0x1 << 2)
#define LED_LAN4_RED_BIT_MASK		(0x1 << 1)
#define LED_LAN4_GREEN_BIT_MASK		(0x1 << 0)

#define LED_WIFI1_RED_ON	(0x0 << 13)
#define LED_WIFI1_RED_OFF 	(0x1 << 13)
#define LED_WIFI1_GREEN_ON	(0x0 << 12)
#define LED_WIFI1_GREEN_OFF (0x1 << 12)

#define LED_WIFI2_RED_ON	(0x0 << 11)
#define LED_WIFI2_RED_OFF 	(0x1 << 11)
#define LED_WIFI2_GREEN_ON	(0x0 << 10)
#define LED_WIFI2_GREEN_OFF (0x1 << 10)

#define LED_LAN1_RED_ON		(0x0 << 7)
#define LED_LAN1_RED_OFF 	(0x1 << 7)
#define LED_LAN1_GREEN_ON	(0x0 << 6)
#define LED_LAN1_GREEN_OFF 	(0x1 << 6)

#define LED_LAN2_RED_ON		(0x0 << 5)
#define LED_LAN2_RED_OFF 	(0x1 << 5)
#define LED_LAN2_GREEN_ON	(0x0 << 4)
#define LED_LAN2_GREEN_OFF 	(0x1 << 4)

#define LED_LAN3_RED_ON		(0x0 << 3)
#define LED_LAN3_RED_OFF 	(0x1 << 3)
#define LED_LAN3_GREEN_ON	(0x0 << 2)
#define LED_LAN3_GREEN_OFF 	(0x1 << 2)

#define LED_LAN4_RED_ON		(0x0 << 1)
#define LED_LAN4_RED_OFF 	(0x1 << 1)
#define LED_LAN4_GREEN_ON	(0x0 << 0)
#define LED_LAN4_GREEN_OFF 	(0x1 << 0)

#define ACTIVE 0
#define TRANS  1
#define FAULT 2

#define power_led_normal 1
#define power_led_upgrade 2

#define LED_3G_BIT_MASK		(0x1 << 15)
#define LED_4G_BIT_MASK		(0x1 << 14)
#define LED_WIFI1_BIT_MASK	(0x3 << 12)
#define LED_WIFI2_BIT_MASK	(0x3 << 10)
#define LED_NMEA_BIT_MASK	(0x3 << 8)
#define LED_LAN1_BIT_MASK	(0x3 << 6)
#define LED_LAN2_BIT_MASK	(0x3 << 4)
#define LED_LAN3_BIT_MASK	(0x3 << 2)
#define LED_LAN4_BIT_MASK	(0x3 << 0)

#define LED_3G_ON			(0x0 << 15)
#define LED_3G_OFF			(0x1 << 15)
#define LED_4G_ON			(0x0 << 14)
#define LED_4G_OFF			(0x1 << 14)

#define LED_WIFI1_ORANGE		(0x0 << 12)
#define LED_WIFI1_RED		(0x1 << 12)
#define LED_WIFI1_GREEN		(0x2 << 12)
#define LED_WIFI1_OFF		(0x3 << 12)

#define LED_WIFI2_ORANGE		(0x0 << 10)
#define LED_WIFI2_RED		(0x1 << 10)
#define LED_WIFI2_GREEN		(0x2 << 10)
#define LED_WIFI2_OFF		(0x3 << 10)

#define LED_NMEA_ORANGE		(0x0 << 8)
#define LED_NMEA_RED			(0x1 << 8)
#define LED_NMEA_GREEN		(0x2 << 8)
#define LED_NMEA_OFF			(0x3 << 8)

#define LED_LAN1_ORANGE		(0x0 << 6)
#define LED_LAN1_RED			(0x1 << 6)
#define LED_LAN1_GREEN		(0x2 << 6)
#define LED_LAN1_OFF			(0x3 << 6)

#define LED_LAN2_ORANGE		(0x0 << 4)
#define LED_LAN2_RED			(0x1 << 4)
#define LED_LAN2_GREEN		(0x2 << 4)
#define LED_LAN2_OFF			(0x3 << 4)

#define LED_LAN3_ORANGE		(0x0 << 2)
#define LED_LAN3_RED			(0x1 << 2)
#define LED_LAN3_GREEN		(0x2 << 2)
#define LED_LAN3_OFF			(0x3 << 2)

#define LED_LAN4_ORANGE		(0x0 << 0)
#define LED_LAN4_RED			(0x1 << 0)
#define LED_LAN4_GREEN		(0x2 << 0)
#define LED_LAN4_OFF			(0x3 << 0)

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
