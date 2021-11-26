#include<stdio.h>
#include<string.h>
#include <stdlib.h>
 
int main()
{
	int i=0;	
	char *a = "BEN";
	unsigned char b[20];
	for(i=0;i<strlen(a);i++){
		b[i]=a[i];
		printf("%c\n",b[i]);
		printf("%x\n",b[i]);
	}
		
	return 0 ;
}
