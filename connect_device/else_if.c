#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main(void)
{
	int a=1,b=1;
	if(a>0&&b>0)
		printf("111\n");
	else if(a>0)
		printf("222\n");
	else if(b>0)
		printf("333\n");
	else
		printf("444\n");
	return;
}
