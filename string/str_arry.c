#include<stdio.h>
#include<string.h>
#include <stdlib.h>
 
char mqtt_data_buf[6][16] = {
	"Host_Name",
	"Host_PortNum",
	"Client_Id",
	"User_Name",
	"User_Pw",
	"Topic",
	"NULL",
};


int main()
{
	int i;
	for(i=0;i<sizeof(mqtt_data_buf)/16;i++){
		if(mqtt_data_buf[i]==NULL)
			printf("it is the end\n");
	}
		//printf("%s\n",mqtt_data_buf[i]);

}
