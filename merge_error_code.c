#include <stdio.h>

#define merge_error_core(module_id,act_id ,error_id) (((module_id) << 16) | ((act_id) << 8) | (error_id) )

void main(void)
{
	int a=20;
	int b=0;
	int c=1;
	char buf[8]="\0";
	snprintf(buf,8,"%02d%02d%02d",a,b,c);
	printf("%s\n",buf);
}
