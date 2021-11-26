#include <stdio.h>
#include <stdlib.h>
#include "libconfig.h"

#define IO_CONF_DATE_PATH "/raymarine/Data/router_conf/IO_CONF_DATE"

typedef struct{
	int id;
	char name[64];
	int level;
	char types[16];
	double volt;
	int status;
	int wake_func;
	int notification;
	char trig_cond_type[8];
	double trig_cond_value;
	char noti_msg[128];
}_IO_CONF_DATE;


int main(void)
{
	int ret;
	config_t cfg;
	config_setting_t *setting;

	char IO_NUM[6];
	int i;
	char *str;

	config_init(&cfg);
  /* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, IO_CONF_DATE_PATH))
	{
		//printf(" %s:%d - %s\n",
		//config_error_file(&cfg),config_error_line(&cfg), config_error_text(&cfg));
		//config_destroy(&cfg);
		return;
	}


	for(i=0;i<8;i++){
		sprintf(IO_NUM,"IO_%d",i+1);
		setting=config_lookup(&cfg,IO_NUM);
		ret=config_setting_lookup_string(setting,"name",&str);
		printf("ret %d\n",ret);
		ret=config_setting_lookup_string(setting,"vlot",&str);
		printf("ret %d\n",ret);
	}

	return 0;
}

