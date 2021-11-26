#include<stdio.h>
#include<malloc.h>
#include<stdbool.h>

typedef struct{
  int b;
  int a;
}info_type;

static info_type info[];

int main(void)
{
	int a = 1;
	int b = 5;

	info[a].b = 1;
	info[a].a = 2;

	info[b].b = 3;
	info[b].a = 4;

	printf("%d %d %d %d \n",info[a].b,info[a].a,info[b].b,info[b].a);

	return 0;
}
