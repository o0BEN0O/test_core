#include<stdio.h>
#include<math.h>
#include<time.h>
int main()
{
//省略
	int i=0;
	time_t b=time(NULL);
	printf("b %ld\n",b);
	sleep(1);
	b=time(NULL);
	printf("b %ld\n",b);
	return 0;
}
