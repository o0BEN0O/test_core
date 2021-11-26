#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define path_db_conf	"/raymarine/Data/router_conf/db.Raymarine"

int read_file(char *file,char *buf,int size)
{
	FILE *fp=NULL;
	int ret;

	memset(buf,0,size);

	//itoa(s,buf,10);
	fp = fopen(file , "rt" );
	if(fp==NULL){
		//JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"open %s failed!\n",file);
		//fclose(fp);
		return -1;
	}

	ret = fread(buf,1,size,fp);/*fread successful: ret == size*/
	if(ret<0){
		//JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"read %s failed!\n",file);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

int write_dns_db_conf(char *ip)
{
	char *point;
	int fd=0;
	char buf[1024];
	int len=0;
	memset(buf,0,1024);
	read_file(path_db_conf,buf,sizeof(buf));
	printf("buf %s\n",buf);
	point=strstr(buf,"RaymarineHub.com. IN A ");
	//len=point-buf-strlen("RaymarineHub.com. IN A ");
	len=strlen(buf)-(point-buf)-strlen("RaymarineHub.com. IN A ");
	printf("len %d\n",len);
	memset(point+strlen("RaymarineHub.com. IN A "),0,len);
	strncpy(point+strlen("RaymarineHub.com. IN A "),ip,strlen(ip));
	strncpy(buf+strlen(buf),"\n",1);
	fd = open(path_db_conf,O_WRONLY|O_TRUNC);
	if(fd < 0){
		perror("open");
		//JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"open %s fail!",path_db_conf);
		close(fd);
		return -1;
		//return WRITE_TO_DB_CONFIG_ERROR;
	}
	write(fd,buf,strlen(buf));
	close(fd);
	return 0;
}


void main(int argc, char *argv[])
{
	char ip[20];
	memset(ip,0,20);
	sscanf(argv[1],"%s",ip);
	printf("ip %s\n",ip);
	write_dns_db_conf(ip);
}
