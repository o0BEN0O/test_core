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
	int a=0;
	int b=0;

	a=59;
	b=17;

again:
	printf("a %d b%d\n",a,b);
	//a=59;
	//b=26;
	if(a>(b+9)){
		//a=59;
		//b=26;
		a-=(b+9);
		printf("a %d b%d\n",a,b);
		goto again;
	}
	return;
	
}
