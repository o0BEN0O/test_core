#include <stdio.h>

typedef struct{
	void *data;
	int i;
}_test;

typedef struct{
        int data;
}_test2;

void point_test(_test *test)
{
	_test2* test2;
	test2=(_test2*)test->data;
	printf("%d\n",test2->data);
}

void main(void)
{
	_test test;
	_test2 test2;
	test2.data=2;
	test.data=(void*)(&test2);
	point_test(&test);
}
