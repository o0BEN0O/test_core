#include<stdio.h>
#include<string.h>
#include <stdlib.h>
char jrd_itoa(int digit)
{
  if (digit>15)
    return ' ';

  if (digit<=9)
    return ('0'+digit);
  if (digit >9 && digit <= 15)
    return ('a'+digit-10);
} 

void print_buf(char *buf)
{
	printf("buf %s\n",buf);
}

int main()
{
	int i=1;
	char a[8]="\0";	
	*a=jrd_itoa(i);
	//printf("111 %s\n",jrd_itoa(i));
	printf("%s\n",a);
	print_buf(a);
}
