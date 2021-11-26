#include <stdio.h>
#include <stdlib.h>

#define tar "20200827010120.tar.bz2"
#define package_path "/raymarine/Data"

int test(char *str)
{
     //char str[512]="\0";
     int ret;
	printf("sizeof %d\n",sizeof(str));
     ret=snprintf(str,128,"tar -jxf %s -C %s",tar,package_path);
//	sprintf(str,"tar -jxf %s -C %s",tar,package_path);
     printf("str=%s ret=%d strlen=%ld\n", str,ret,strlen(str));
     return 0;
}

int main()
{        
    char str[512]="\0";
	printf("%d\n",sizeof(str));
    test(str);
	printf("str %s\n",str);
}
