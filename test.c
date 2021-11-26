#include <stdlib.h>
#include <stdio.h>

typedef struct{
	unsigned int ad_channel_id;
	unsigned long channel_val;
}_ad_date_format;

void main(void)
{
	_ad_date_format ad_date;
	unsigned int rx_date[3]={0x08,0x11,0x13};
	unsigned int tmp[3];
	tmp[0]=rx_date[0];
	tmp[1]=rx_date[1];
	tmp[2]=rx_date[2];
	ad_date.ad_channel_id=rx_date[0];
	ad_date.channel_val=(rx_date[2]<<8|rx_date[1]);
	double val = (double)(ad_date.channel_val)/1000;
	printf("%lf\n",val);
	printf("0x%x 0x%x\n",ad_date.ad_channel_id,ad_date.channel_val);
}
