#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include <string.h>

void read_ip(void)
{
	FILE *fp=NULL;
	char buf[20];
	int i=0;
	memset(buf,0,20);
	fp=fopen("/tmp/qca9563_ip_addr","rt");
	fread(buf,1,20,fp);
	while(strlen(buf)==0){
		usleep(10);
		fread(buf,1,20,fp);
		i++;
	}
	printf("i %d ip %s\n",i,buf);
	fclose(fp);
	system("rm -rf /tmp/qca9563_ip_addr");
	return;
}
void get_ip(void)
{
	int i=0;
	while(access("/tmp/qca9563_ip_addr",0)!=0){
		i++;
		usleep(1);
	}
	//printf("i %d\n",i);
	read_ip();
}


int main(void)
{
	FILE *fd;
	char buf[20];
	int pid;
	fd=fopen("/tmp/test_pid","rt");
	fread(buf,1,sizeof(buf),fd);
	fclose(fd);
	pid=atoi(buf);
	printf("pid %d\n",pid);
	kill(pid,SIGRTMIN+1);
	get_ip();
	return 0;
}
