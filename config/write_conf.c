#include <stdio.h>
#include <stdlib.h>
#include "libconfig.h"

#define IO_CONF_DATE_PATH "/raymarine/Data/router_conf/IO_CONF_DATE"

/* This example constructs a new configuration in memory and writes it to
 * 'newconfig.cfg'.
 */

void main(void)
{
	int ret;
	config_t cfg;
	config_setting_t *root,*setting;

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
#if 0
	root=config_lookup(&cfg,"IO_5");
	setting=config_setting_lookup(root,"status");
	config_setting_set_int(setting,1);
	setting=config_setting_lookup(root,"status");
	config_setting_set_int(setting,1);
#endif
	root=config_lookup(&cfg,"IO_1");
	setting=config_setting_lookup(root,"name");
	config_setting_set_string(setting,"test");
	setting=config_setting_lookup(root,"types");
	config_setting_set_int(setting,1);
	setting=config_setting_lookup(root,"wake_func");
	config_setting_set_int(setting,1);

	config_write_file(&cfg, IO_CONF_DATE_PATH);

	return;
}

