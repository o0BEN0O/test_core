#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


#define fixed_data_path "/home/ben/ben/4g_router/test/fixed_datas"

pthread_mutex_t mutex;


typedef struct {
	char Mode[128];
	char ipq_version[64];
	char serial_number[128] ;
	char IMEI[32];
	long Operating_hours;//unit sec
	char imx_eth_mac[128];
	char ip_addr[128];
}_data_fixed;

void printf_data(_data_fixed *data_fixed)
{
	printf("%s\n",data_fixed->Mode);
	printf("%s\n",data_fixed->ipq_version);
	printf("%s\n",data_fixed->serial_number);
	printf("%s\n",data_fixed->IMEI);
	printf("%ld\n",data_fixed->Operating_hours);
	printf("%s\n",data_fixed->imx_eth_mac);
	printf("%s\n",data_fixed->ip_addr);
}

void write_file(_data_fixed *data_fixed)
{
	 pthread_mutex_lock(&mutex);
	FILE *fp_fixed_data = fopen(fixed_data_path,"wb+");/*must access file ,else open fail*/
	if(fp_fixed_data == NULL){
		printf("open %s failed\n",fixed_data_path);
		fclose(fp_fixed_data);
		return;
	}
	sleep(1);
	fwrite(data_fixed,sizeof(_data_fixed),1,fp_fixed_data);
	fclose(fp_fixed_data);
	pthread_mutex_unlock(&mutex);
}

_data_fixed *read_from_hub_info(void)
{
	char *buf;
	int len;
	FILE* fp_fixed_data = fopen(fixed_data_path,"r+b");/*must access file ,else open fail*/
	_data_fixed *data_fixed;
	if(fp_fixed_data == NULL){
		printf("open %s failed\n",fixed_data_path);
		fclose(fp_fixed_data);
		return;
	}
	//printf("sizeof %d\n",sizeof(_data_fixed));
	buf = malloc(sizeof(_data_fixed));
	fread(buf,sizeof(_data_fixed),1,fp_fixed_data);
	data_fixed = (_data_fixed *) buf;
	fclose(fp_fixed_data);
	free(buf);
	return data_fixed;
}


void func1(void)
{
	while(1){
	_data_fixed data_fixed1;
	_data_fixed *data_fixed;
	printf("enter func1\n");
	strncpy(data_fixed1.Mode,"hubrouter",strlen("hubrouter"));
	strncpy(data_fixed1.ipq_version,"11111",strlen("11111"));
	strncpy(data_fixed1.serial_number,"101010",strlen("101010"));
	strncpy(data_fixed1.IMEI,"IEMI11",strlen("IEMI11"));
	data_fixed1.Operating_hours = 1000;
	strncpy(data_fixed1.imx_eth_mac,"11:11:11:00",strlen("11:11:11:00"));
	write_file(&data_fixed1);
	data_fixed = read_from_hub_info();
	printf_data(data_fixed);
	}
}

void func2(void)
{
	while(1){
		sleep(2);
	_data_fixed *data_fixed2;
		printf("enter func2\n");
	data_fixed2 = read_from_hub_info();
	strncpy(data_fixed2->ip_addr,"192.18.248.100",strlen("192.18.248.100"));
	write_file(data_fixed2);
	printf_data(data_fixed2);
	}
}


void main(void)
{
	pthread_t a;
	pthread_t b;
	pthread_attr_t attr;
	pthread_mutex_init(&mutex,NULL);

	if (pthread_attr_init(&attr) < 0)
	{
	 	printf(" init thread attribute failed\n");
	}

	if (pthread_attr_setstacksize (&attr, 64*1024) != 0)
	{
		printf(" set stack size failed\n");
	  	perror("pthread_attr_setstacksize:");
	}

	if (pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED) < 0)
	{
		printf(" set setdetachstate failed\n");
		perror("pthread_attr_setdetachstate:");
	}

	if (pthread_create(&a, &attr, func1, NULL) < 0)
	{
		printf(" set pthread_create failed\n");
	}

	if (pthread_create(&a, &attr, func2, NULL) < 0)
	{
		printf(" set pthread_create failed\n");
	}

	while(1){
		sleep(1);
	}

}


