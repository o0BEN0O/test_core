#include <stdio.h>
#include <string.h>



typedef unsigned int uint8;
typedef unsigned short uint16;
typedef unsigned long uint32; 

typedef struct{
	uint8 ad_channel_id;
	uint8 warning_type;
}_input_warning_params;

void send(void *params)
{
	int index=0;
	uint8 data[2];
	memset(data,0,2);
	data[index++]=(*((uint32*)(params)) & 0xff);
	data[index++]=(*((uint32*)(params))>>8 & 0xff);
	data[index++]=(*((uint32*)(params))>>16 & 0xff);
	data[index++]=(*((uint32*)(params))>>24 & 0xff);
	printf("%02x %02x %02x %02x\n",data[0],data[1],data[2],data[3]);
}

void cal_size(void)
{
	_input_warning_params params[4];
	//printf("sizeof params %d\n",sizeof(params));
}

void input_params(double trig_cond_value,int trig_cond_type)
{
	uint32 input_value=0x0;
	if(trig_cond_type==0){
		input_value|=(int)(trig_cond_value*10);
		input_value|=0xffff0000;
	}else{
		input_value|=(((int)(trig_cond_value*10))<<16);
		input_value|=0xffff;
	}
	printf("0x%08lx\n",input_value);
	send(&input_value);
}

void at_send(char *at_cmd)
{
	int i=0;
	int index=0;
	uint8 data[256];
	data[index++]=0x03;
	for(i=0;i<strlen(at_cmd);i++){
		data[index++]=at_cmd[i];
		printf("%c",data[i+1]);
	}
	printf("index %d\n",index);
}

void main(void)
{
	int time=0;
	send(&time);
	//at_send("AT+QSCLK=1");
	//cal_size();

	//input_params(12.3,0);
	//input_params(12.3,1);

	//input_params(28.6,0);
	//input_params(28.6,1);
}
