#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "json.h"
#include "json_object.h"

int main(int argc, char *argv[])
{
	int ret=0;
	int i=0;
	json_object *obj=NULL;
	json_object *params_object=NULL;
	json_object *array_object=NULL;
	unsigned int counter=(argc-1)/2;
	char *json_data_buf;

	obj = json_object_new_object();
	array_object = json_object_new_array();

	printf("counter [%d]\n",counter);

	for(i=0;i<counter;i++){
		params_object = json_object_new_object();
		json_object_object_add(params_object,"IPAddress",json_object_new_string(argv[i+1]));
		json_object_object_add(params_object,"MACAddress",json_object_new_string(argv[i+2]));
		json_object_array_add(array_object,params_object);
	}

	params_object = json_object_new_object();
	json_object_object_add(params_object,"Eth0DevList",array_object);

	json_object_object_add(obj,"jsonrpc",json_object_new_string("2.0"));

	json_object_object_add(obj,"method",json_object_new_string("GetEth0ConnectedDevices"));

	json_object_object_add(obj,"params",params_object);

	json_object_object_add(obj,"id",json_object_new_string("9.1"));

	json_data_buf=(char *)json_object_to_json_string(obj);

	return 0;

}

