#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define filename "/raymarine/Data/20200828023923.tar.bz2"

typedef struct{
	int a;
	int b;
	int c;
}_data;

typedef struct{
	char *file;
	_data *data;
}_parm;

int test(_parm *parm)
{
	printf("filename %s\n",parm->file);
	printf("data %d %d %d\n",parm->data->a,parm->data->b,parm->data->c);
}

int main()
{
	_parm *parm;
	_data *data;
	data->a=1;
	data->b=2;
	data->c=3;
	parm->file=(char *)malloc(sizeof(char)*strlen(filename));
	memcpy(parm->file,filename,strlen(filename));
	memcpy(parm->data,data,sizeof(_data));
	test(parm);
}

