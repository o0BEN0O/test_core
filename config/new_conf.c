/* ----------------------------------------------------------------------------
   libconfig - A library for processing structured configuration files
   Copyright (C) 2005-2018  Mark A Lindner

   This file is part of libconfig.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include "libconfig.h"

/* This example constructs a new configuration in memory and writes it to
 * 'newconfig.cfg'.
 */

int main(int argc, char **argv)
{
  static const char *output_file = "/raymarine/Data/IO_CONF_DATE";
  config_t cfg;
  config_setting_t *root, *setting, *group, *array;
  int i;
  char IO_NUM[6];
  char io_name[20];

  config_init(&cfg);
  root = config_root_setting(&cfg);

#if 1
	for(i=0;i<4;i++){
		sprintf(IO_NUM,"IO_%d",i+1);
		sprintf(io_name,"channel %d",i+1);
		group = config_setting_add(root, IO_NUM, CONFIG_TYPE_GROUP);
		setting = config_setting_add(group, "id", CONFIG_TYPE_INT);
   		config_setting_set_int(setting, i+1);
  		setting = config_setting_add(group, "name", CONFIG_TYPE_STRING);
   		config_setting_set_string(setting, io_name);
		setting = config_setting_add(group, "types", CONFIG_TYPE_INT);
   		config_setting_set_string(setting, 0);
		setting = config_setting_add(group, "status", CONFIG_TYPE_INT);
   		config_setting_set_int(setting, 0);
		setting = config_setting_add(group, "wake_func", CONFIG_TYPE_INT);
   		config_setting_set_int(setting, 0);
		setting = config_setting_add(group, "notification", CONFIG_TYPE_INT);
   		config_setting_set_int(setting, 0);
		setting = config_setting_add(group, "trig_cond_type", CONFIG_TYPE_INT);
   		config_setting_set_string(setting, 0);
		setting = config_setting_add(group, "trig_cond_value", CONFIG_TYPE_FLOAT);
   		config_setting_set_int(setting, 0);
		setting = config_setting_add(group, "noti_msg", CONFIG_TYPE_STRING);
   		config_setting_set_string(setting, NULL);
	}

	for(i=4;i<8;i++){
		sprintf(IO_NUM,"IO_%d",i+1);
		sprintf(io_name,"channel %d",i+1);
		group = config_setting_add(root, IO_NUM, CONFIG_TYPE_GROUP);
		setting = config_setting_add(group, "id", CONFIG_TYPE_INT);
   		config_setting_set_int(setting, i+1);
  		setting = config_setting_add(group, "name", CONFIG_TYPE_STRING);
   		config_setting_set_string(setting, io_name);
		setting = config_setting_add(group, "types", CONFIG_TYPE_INT);
   		config_setting_set_string(setting, 0);
		setting = config_setting_add(group, "status", CONFIG_TYPE_INT);
   		config_setting_set_int(setting, NULL);
		setting = config_setting_add(group, "wake_func", CONFIG_TYPE_INT);
   		config_setting_set_int(setting, NULL);
		setting = config_setting_add(group, "notification", CONFIG_TYPE_INT);
   		config_setting_set_int(setting, NULL);
		setting = config_setting_add(group, "trig_cond_type", CONFIG_TYPE_INT);
   		config_setting_set_string(setting, 0);
		setting = config_setting_add(group, "trig_cond_value", CONFIG_TYPE_FLOAT);
   		config_setting_set_int(setting, NULL);
		setting = config_setting_add(group, "noti_msg", CONFIG_TYPE_STRING);
   		config_setting_set_string(setting, NULL);
	}
#endif

#if 0
  /* Add some settings to the configuration. */
  group = config_setting_add(root, "address", CONFIG_TYPE_GROUP);

  setting = config_setting_add(group, "street", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "1 Woz Way");

  setting = config_setting_add(group, "city", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, "San Jose");

  setting = config_setting_add(group, "state", CONFIG_TYPE_STRING);
  config_setting_set_string(setting, NULL);

  setting = config_setting_add(group, "zip", CONFIG_TYPE_INT);
  config_setting_set_int(setting, NULL);

  array = config_setting_add(root, "numbers", CONFIG_TYPE_ARRAY);

  for(i = 0; i < 10; ++i)
  {
    setting = config_setting_add(array, NULL, CONFIG_TYPE_INT);
    config_setting_set_int(setting, 10 * i);
  }
#endif

#if 0
	setting = config_setting_add(root, "Host_Name", CONFIG_TYPE_STRING);
	config_setting_set_int(setting,NULL);

	setting = config_setting_add(root, "Host_PortNum", CONFIG_TYPE_STRING);
        config_setting_set_int(setting,NULL);

        setting = config_setting_add(root, "Client_Id", CONFIG_TYPE_STRING);
        config_setting_set_int(setting,NULL);

        setting = config_setting_add(root, "User_Name", CONFIG_TYPE_STRING);
        config_setting_set_int(setting,NULL);

        setting = config_setting_add(root, "User_Pw", CONFIG_TYPE_STRING);
        config_setting_set_int(setting,NULL);

        setting = config_setting_add(root, "Topic", CONFIG_TYPE_STRING);
        config_setting_set_int(setting,NULL);
#endif

  /* Write out the new configuration. */
  if(! config_write_file(&cfg, output_file))
  {
    fprintf(stderr, "Error while writing file.\n");
    config_destroy(&cfg);
    return(EXIT_FAILURE);
  }

  fprintf(stderr, "New configuration successfully written to: %s\n",
          output_file);

  config_destroy(&cfg);
  return(EXIT_SUCCESS);
}

/* eof */
