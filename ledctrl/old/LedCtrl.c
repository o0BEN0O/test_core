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

static void  hc595_write(int data)
{
	int clk_fd,sda_fd,u8i = 0,temp;
	
	clk_fd = open(HC595_CLK_VAL,O_WRONLY | O_SYNC);
	sda_fd = open(HC595_SDA_VAL,O_WRONLY | O_SYNC);
	//lt_fd = open(HC595_LT_VAL,O_WRONLY | O_SYNC);
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

	//write(lt_fd,GPIO_PIN_OUTPUT_0,sizeof(GPIO_PIN_OUTPUT_0));
	//write(lt_fd,GPIO_PIN_OUTPUT_1,sizeof(GPIO_PIN_OUTPUT_1));
	close(clk_fd);
	close(sda_fd);
	//close(lt_fd);
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
		printf("clk_fd ERR: export pin error \n");
	}
	write(fd,HC595_CLK_NO,sizeof(HC595_CLK_NO));
	write(fd,HC595_SDA_NO,sizeof(HC595_SDA_NO));
	write(fd,HC595_LT_NO,sizeof(HC595_LT_NO));
	write(fd,GPIO1_8_PW_LED,sizeof(HC595_LT_NO));
	close(fd);

	fd = open(HC595_CLK_DIR,O_WRONLY | O_SYNC);
	if(fd == -1){
		printf("clk_fd clk_fdERR: open direction error \n");
	}
	write(fd,GPIO_PIN_DIR_OUT,sizeof(GPIO_PIN_DIR_OUT));
	close(fd);
		
	fd = open(HC595_SDA_DIR,O_WRONLY | O_SYNC);
	if(fd == -1){
		printf("sda_fd sda_fdERR: open direction error \n");
	}
	write(fd,GPIO_PIN_DIR_OUT,sizeof(GPIO_PIN_DIR_OUT));
	close(fd);

	fd = open(HC595_LT_DIR,O_WRONLY | O_SYNC);
	if(fd == -1){
		printf("lt_fd lt_fdERR: open direction error \n");
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

void led_set(int date)
{
	led_control(LED_WIFI2_RED_ON,LED_WIFI2_RED_BIT_MASK);
}

int main(void)
{
	struct itimerval tick;
	init_led_control();

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
		//led_set(1);
		sleep(1);
	}
}