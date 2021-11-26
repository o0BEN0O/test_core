/*strncmpexample*/
#include<stdio.h>
#include<string.h>

#define str1 "/dev/mmcblk2p4"
char str2[128]="\0";

int main()
{
	int ret;
	strncpy(str2,str1,strlen(str1));
	ret=strncmp(str2,str1,strlen(str1));
	printf("ret %d\n",ret);
	return ret;
}
