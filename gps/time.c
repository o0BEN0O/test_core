
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>



int main(void)
{

	time_t t;
	t = time(NULL);

	int ii = time(&t);
	struct tm *local = localtime(&t);
	printf("ii = %d\n", ii);
	printf("%d-%d-%d-%d-%d-%d\n",local->tm_year+1900,local->tm_mon+1,local->tm_mday,local->tm_hour,local->tm_min,local->tm_sec);
	return 0;
	/*
	ii = 1516020076
	请按任意键继续. . .
	*/
}
