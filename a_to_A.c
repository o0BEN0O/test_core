#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
int a_to_A(char *p)  
{    
    while(*p!='\0'){
        if(*p>='a'&&*p<='z')
            *p-=32;
        p++;
    }
    return 0;  
}  

void main(void)
{
	char a[20];
	memset(a,0,20);
	memcpy(a,"00:a0:c6:eb:5b:41",17);
	a_to_A(a);
	printf("%s\n",a);
}
