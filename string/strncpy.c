#include<stdio.h>
#include<string.h>
#include <stdlib.h>
 
int main()
{
    char *a = "12345" ;
    char *b ; 
    b = malloc(strlen(a)+4);
    memset(b,0,strlen(a)) ;
    strncpy(b,"abcdefg",sizeof("abcdefg")); 
    strncpy(b,a,strlen(a));
    strncpy(b+sizeof(a),"\0",1);
    printf("%s\n",b);
    return 0 ;
 }
