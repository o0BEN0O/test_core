#include <stdbool.h>
#include <string.h>
#include <stdio.h>   
#include <errno.h>  
#include <sys/stat.h>
#include <fcntl.h>  
#include <termios.h>   
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include "LedCtrl.h"
//#include "jrd_oem.h"

#define GPIO_EXPORT					"/sys/class/gpio/export"
#define GPIO_UNEXPORT				"/sys/class/gpio/unexport"

#define GPIO1_8_PW_LED				"8"
#define HC595_CLK_NO				"86"
#define HC595_SDA_NO				"87"
#define HC595_LT_NO				"88"

#define HC595_CLK_DIR			"/sys/class/gpio/gpio86/direction"
#define HC595_SDA_DIR			"/sys/class/gpio/gpio87/direction"
#define HC595_LT_DIR				"/sys/class/gpio/gpio88/direction"

#define GPIO1_8_PW_LED_VAL 		"/sys/class/gpio/gpio8/value"
#define HC595_CLK_VAL			"/sys/class/gpio/gpio86/value"
#define HC595_SDA_VAL			"/sys/class/gpio/gpio87/value"
#define HC595_LT_VAL				"/sys/class/gpio/gpio88/value"

#define GPIO_PIN_DIR_OUT			"out"
#define GPIO_PIN_OUTPUT_1		"1"
#define GPIO_PIN_OUTPUT_0		"0"


int iLedControlValue=0xFFFF;

int led_nmea_status=0;
int led_nmea_timer=0;
int led_nmea_cycle=0;
#define LED_NMEA_ALL_FINE 0
#define LED_NMEA_BUS_NO_CONNECT 1
#define LED_NMEA_BUS_NO_DATE 2

int led_lte_status = 0;
int led_lte_timer = 0;
int led_lte_cycle = 0;
int led_lte_over_timer=0;/*CONNECTING:100, CONNECTED:1450,TRANS:50*/
int led_lte_total_timer=0;/*CONNECTING:200, CONNECTED:1500,TRANS:100*/
typedef enum{
	LED_LTE_CONNECTING_4G=0,
	LED_LTE_CONNECTED_4G,
	LED_LTE_4G_TRANS,
	LED_LTE_CONNECTING_3G,
	LED_LTE_CONNECTED_3G,
	LED_LTE_3G_TRANS,
	LED_LTE_SIG_FAULT,
	LED_LTE_NO_SIM,
}EM_LED_LTE;

int led_cloud_status=0;
int led_cloud_timer = 0;
#define LED_CLOUD_CONNECTED 0
#define LED_CLOUD_NO_CONNECTED 1

int led_wifi1_status=0;
int led_wifi1_timer=0;
#define LED_WIFI1_ACT 0
#define LED_WIFI1_TRANS 1
#define LED_WIFI1_FAULT 2
#define LED_OFF_WIFI1 3

int led_wifi2_status=0;
int led_wifi2_timer=0;
#define LED_WIFI2_ACT 0
#define LED_WIFI2_TRANS 1
#define LED_WIFI2_FAULT 2
#define LED_OFF_WIFI2 3


static void  hc595_write(int data)
{
	int clk_fd,sda_fd,lt_fd,u8i = 0,temp;
	
	clk_fd = open(HC595_CLK_VAL,O_WRONLY | O_SYNC);
	sda_fd = open(HC595_SDA_VAL,O_WRONLY | O_SYNC);
	lt_fd = open(HC595_LT_VAL,O_WRONLY | O_SYNC);
	for(u8i = 0;u8i < 8;u8i++)
	{
		write(clk_fd,GPIO_PIN_OUTPUT_0,sizeof(GPIO_PIN_OUTPUT_0));
		temp = data & 0x80;
		if(temp == 0x80)
		{
			write(sda_fd,GPIO_PIN_OUTPUT_1,sizeof(GPIO_PIN_OUTPUT_1));
		}
		else
		{
			write(sda_fd,GPIO_PIN_OUTPUT_0,sizeof(GPIO_PIN_OUTPUT_0));
		}
		data = data<<1;
		write(clk_fd,GPIO_PIN_OUTPUT_1,sizeof(GPIO_PIN_OUTPUT_1));
	}

	write(lt_fd,GPIO_PIN_OUTPUT_0,sizeof(GPIO_PIN_OUTPUT_0));
	write(lt_fd,GPIO_PIN_OUTPUT_1,sizeof(GPIO_PIN_OUTPUT_1));
	close(clk_fd);
	close(sda_fd);
	close(lt_fd);
}

void power_led_control(char *on_off)
{
	int fd;
	fd = open(GPIO1_8_PW_LED_VAL,O_WRONLY | O_SYNC);
	write(fd,on_off,sizeof(on_off));
	close(fd);
}

void init_led_control(void)
{
	int fd;
	
	fd = open(GPIO_EXPORT,O_WRONLY | O_SYNC);
	if(fd == -1)
	{
		//JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW,"clk_fd ERR: export pin error \n");
	}
	write(fd,HC595_CLK_NO,sizeof(HC595_CLK_NO));
	write(fd,HC595_SDA_NO,sizeof(HC595_SDA_NO));
	write(fd,HC595_LT_NO,sizeof(HC595_LT_NO));
	write(fd,GPIO1_8_PW_LED,sizeof(HC595_LT_NO));
	close(fd);

	fd = open(HC595_CLK_DIR,O_WRONLY | O_SYNC);
	if(fd == -1){
		//JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW,"clk_fd clk_fdERR: open direction error \n");
	}
	write(fd,GPIO_PIN_DIR_OUT,sizeof(GPIO_PIN_DIR_OUT));
	close(fd);
		
	fd = open(HC595_SDA_DIR,O_WRONLY | O_SYNC);
	if(fd == -1){
		//JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW,"sda_fd sda_fdERR: open direction error \n");
	}
	write(fd,GPIO_PIN_DIR_OUT,sizeof(GPIO_PIN_DIR_OUT));
	close(fd);

	fd = open(HC595_LT_DIR,O_WRONLY | O_SYNC);
	if(fd == -1){
		//JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW,"lt_fd lt_fdERR: open direction error \n");
	}
	write(fd,GPIO_PIN_DIR_OUT,sizeof(GPIO_PIN_DIR_OUT));
	close(fd);
}

void led_control(int iLedStatus,int iLedBitMask)
{
	iLedControlValue=(iLedControlValue&(~iLedBitMask))|iLedStatus;
	
	hc595_write(iLedControlValue&0xFF);
	hc595_write((iLedControlValue>>8)&0xFF);
}

void led_nmea_ctrl(void)
{
	switch(led_nmea_status){
	case LED_NMEA_ALL_FINE:
		if(led_nmea_timer<50)
			led_control(LED_NMEA_OFF,LED_NMEA_BIT_MASK);
		else if(led_nmea_timer >= 50||led_nmea_timer<1500)
			led_control(LED_NMEA_GREEN_ON,LED_NMEA_BIT_MASK);
		led_nmea_timer++;
		if(led_nmea_timer==1500)
			led_nmea_timer=0;
		return;

	case LED_NMEA_BUS_NO_CONNECT:
		//led_nmea_timer=0;
		if(led_nmea_timer < 25)
			led_control(LED_NMEA_RED_ON,LED_NMEA_BIT_MASK);
		else if(led_nmea_timer >= 25 && led_nmea_timer<80)
			led_control(LED_NMEA_OFF,LED_NMEA_BIT_MASK);
		else if(led_nmea_timer >= 80 && led_nmea_timer<105)
			led_control(LED_NMEA_RED_ON,LED_NMEA_BIT_MASK);
		else if(led_nmea_timer >= 105 && led_nmea_timer<400)
			led_control(LED_NMEA_OFF,LED_NMEA_BIT_MASK);
		led_nmea_timer++;
		if(led_nmea_timer==400)
			led_nmea_timer=0;
		return;

	case LED_NMEA_BUS_NO_DATE:
		if(led_nmea_timer<480){
			if((led_nmea_cycle*80)<=led_nmea_timer && led_nmea_timer < (80*led_nmea_cycle+25)){
				//printf("led_nmea_cycle =%d\n",led_nmea_cycle);
				led_control(LED_NMEA_RED_ON,LED_NMEA_BIT_MASK);
			}else{
				led_control(LED_NMEA_OFF,LED_NMEA_BIT_MASK);
				//printf("off =%d\n",led_nmea_cycle);
			}
		}
		if(led_nmea_timer<505&&led_nmea_timer>480)
			led_control(LED_NMEA_RED_ON,LED_NMEA_BIT_MASK);
		if(led_nmea_timer<900&&led_nmea_timer>505)
			led_control(LED_NMEA_OFF,LED_NMEA_BIT_MASK);
		led_nmea_timer++;

		if(led_nmea_timer%80==0&&led_nmea_timer<480)
			led_nmea_cycle++;
	
		if(led_nmea_timer==900){
			led_nmea_timer=0;
			led_nmea_cycle=0;
		}
		return;
	}
}

void led_lte_ctrl(void)
{
	switch(led_lte_status){
	case LED_LTE_CONNECTING_4G:
	case LED_LTE_CONNECTED_4G:
	case LED_LTE_4G_TRANS:
		if(led_lte_timer<led_lte_over_timer)
			led_control(LED_LTE_4G_ON,LED_LTE_BIT_MASK);
		else
			led_control(LED_LTE_OFF,LED_LTE_BIT_MASK);
		led_lte_timer++;
		if(led_lte_timer==led_lte_total_timer){
			led_lte_timer=0;
			return;
		}
		return;

	case LED_LTE_CONNECTING_3G:
	case LED_LTE_CONNECTED_3G:
	case LED_LTE_3G_TRANS:
		if(led_lte_timer<led_lte_over_timer)
			led_control(LED_LTE_3G_ON,LED_LTE_BIT_MASK);
		else
			led_control(LED_LTE_OFF,LED_LTE_BIT_MASK);
		led_lte_timer++;
		if(led_lte_timer==led_lte_total_timer){
			led_lte_timer=0;
			return;
		}
		return;

	case LED_LTE_SIG_FAULT:
		if(led_lte_timer < 25)
			led_control(LED_LTE_NO_SIG,LED_LTE_BIT_MASK);
		else if(led_lte_timer >= 25 && led_lte_timer<80)
			led_control(LED_LTE_OFF,LED_LTE_BIT_MASK);
		else if(led_lte_timer >= 80 && led_lte_timer<105)
			led_control(LED_LTE_NO_SIG,LED_LTE_BIT_MASK);
		else if(led_lte_timer >= 105 && led_lte_timer<400)
			led_control(LED_LTE_OFF,LED_LTE_BIT_MASK);
		led_lte_timer++;
		if(led_lte_timer==400)
			led_lte_timer=0;
		return;

	case LED_LTE_NO_SIM:
		if(led_lte_timer < 25)
			led_control(LED_LTE_NO_SIG,LED_LTE_BIT_MASK);
		else if(led_lte_timer >= 25 && led_lte_timer<80)
			led_control(LED_LTE_OFF,LED_LTE_BIT_MASK);
		else if(led_lte_timer >= 80 && led_lte_timer<105)
			led_control(LED_LTE_NO_SIG,LED_LTE_BIT_MASK);
		else if(led_lte_timer >= 105 && led_lte_timer<160)
			led_control(LED_LTE_OFF,LED_LTE_BIT_MASK);
		else if(led_lte_timer >= 160 && led_lte_timer<185)
			led_control(LED_LTE_NO_SIG,LED_LTE_BIT_MASK);
		else
			led_control(LED_LTE_OFF,LED_LTE_BIT_MASK);
		led_lte_timer++;
		if(led_lte_timer==500)
			led_lte_timer=0;
		return;
	}
}

void led_cloud_ctrl(void)
{
	switch(led_cloud_status){
	case LED_CLOUD_CONNECTED:
		if(led_cloud_timer<1450)
			led_control(LED_CLOUD_GREEN_ON,LED_CLOUD_BIT_MASK);
		else
			led_control(LED_CLOUD_OFF,LED_CLOUD_BIT_MASK);
		led_cloud_timer++;
		if(led_cloud_timer==1500)
			led_cloud_timer=0;
		return;
	case LED_CLOUD_NO_CONNECTED:
		led_control(LED_CLOUD_OFF,LED_CLOUD_BIT_MASK);
		return;
	}
}

void led_wifi1_ctrl(void)
{
	switch(led_wifi1_status){
	case LED_WIFI1_ACT:
		if(led_wifi1_timer<1450)
			led_control(LED_WIFI1_GREEN_ON,LED_WIFI1_BIT_MASK);
		else
			led_control(LED_WIFI1_OFF,LED_WIFI1_BIT_MASK);
		led_wifi1_timer++;
		if(led_wifi1_timer==1500)
			led_wifi1_timer=0;
		return;

	case LED_WIFI1_TRANS:
		if(led_wifi1_timer<50)
			led_control(LED_WIFI1_GREEN_ON,LED_WIFI1_BIT_MASK);
		else
			led_control(LED_WIFI1_OFF,LED_WIFI1_BIT_MASK);
		led_wifi1_timer++;
		if(led_wifi1_timer==100)
			led_wifi1_timer=0;
		return;

	case LED_WIFI1_FAULT:
		if(led_wifi1_timer < 25)
			led_control(LED_WIFI1_RED_ON,LED_WIFI1_BIT_MASK);
		else if(led_wifi1_timer >= 25 && led_wifi1_timer<80)
			led_control(LED_WIFI1_OFF,LED_WIFI1_BIT_MASK);
		else if(led_wifi1_timer >= 80 && led_wifi1_timer<105)
			led_control(LED_WIFI1_RED_ON,LED_WIFI1_BIT_MASK);
		else if(led_wifi1_timer >= 105 && led_wifi1_timer<400)
			led_control(LED_WIFI1_OFF,LED_WIFI1_BIT_MASK);
		led_wifi1_timer++;
		if(led_wifi1_timer==400)
			led_wifi1_timer=0;
		return;

	case LED_OFF_WIFI1:
		led_control(LED_WIFI1_OFF,LED_WIFI1_BIT_MASK);
		return;
	}
}

void led_wifi2_ctrl(void)
{
	switch(led_wifi2_status){
	case LED_WIFI2_ACT:
		if(led_wifi2_timer<1450)
			led_control(LED_WIFI2_GREEN_ON,LED_WIFI2_BIT_MASK);
		else
			led_control(LED_WIFI2_OFF,LED_WIFI2_BIT_MASK);
		led_wifi2_timer++;
		if(led_wifi2_timer==1500)
			led_wifi2_timer=0;
		return;

	case LED_WIFI2_TRANS:
		if(led_wifi2_timer<50)
			led_control(LED_WIFI2_GREEN_ON,LED_WIFI2_BIT_MASK);
		else
			led_control(LED_WIFI2_OFF,LED_WIFI2_BIT_MASK);
		led_wifi2_timer++;
		if(led_wifi2_timer==100)
			led_wifi2_timer=0;
		return;

	case LED_WIFI2_FAULT:
		if(led_wifi2_timer < 25)
			led_control(LED_WIFI2_RED_ON,LED_WIFI2_BIT_MASK);
		else if(led_wifi2_timer >= 25 && led_wifi2_timer<80)
			led_control(LED_WIFI2_OFF,LED_WIFI2_BIT_MASK);
		else if(led_wifi2_timer >= 80 && led_wifi2_timer<105)
			led_control(LED_WIFI2_RED_ON,LED_WIFI2_BIT_MASK);
		else if(led_wifi2_timer >= 105 && led_wifi2_timer<400)
			led_control(LED_WIFI2_OFF,LED_WIFI2_BIT_MASK);
		led_wifi2_timer++;
		if(led_wifi2_timer==400)
			led_wifi2_timer=0;
		return;

	case LED_OFF_WIFI2:
		led_control(LED_WIFI2_OFF,LED_WIFI2_BIT_MASK);
		return;
	}
}


void led_set(int date){
	led_lte_ctrl();
#if 0
	led_nmea_ctrl();
	led_cloud_ctrl();
	led_wifi1_ctrl();
	led_wifi2_ctrl();
#endif
}

int main(void)
{
	//turn 0;
#if 1
	struct itimerval tick;
	
	init_led_control();
		led_control(LED_NMEA_GREEN_ON,LED_NMEA_BIT_MASK);
	led_control(LED_WIFI1_GREEN_ON,LED_WIFI1_BIT_MASK);
	led_control(LED_WIFI2_GREEN_ON,LED_WIFI2_BIT_MASK);

	//d_control(LED_LTE_NO_SIM,LED_LTE_BIT_MASK);
	led_control(LED_CLOUD_GREEN_ON,LED_CLOUD_BIT_MASK);
	led_nmea_status=LED_NMEA_BUS_NO_DATE;
	led_lte_status=LED_LTE_NO_SIM;
	led_cloud_status=LED_CLOUD_CONNECTED;
	led_wifi1_status=LED_WIFI1_OFF;
	led_wifi2_status=LED_WIFI2_OFF;
#endif
#if 1
	signal(SIGALRM, led_set);
	memset(&tick, 0, sizeof(tick));

	//Timeout to run first time
	tick.it_value.tv_sec = 0;
	tick.it_value.tv_usec = 10000;

	//After first, the Interval time for clock
	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = 10000;

	if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
	{
		perror("Set timer failed!\n");
	}
#endif

	while(1){
		sleep(1);
	}

	return 0;
}
