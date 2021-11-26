#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/vfs.h>

#define RaymarineFirmware "/raymarine/Data/firmware/mfd_upgrade.tar.bz2"
#define StartingUpgradeFlag "/raymarine/Data/Upgrade_FLAG/Start_Ugrading_Flag"

void fail_return(const char *msg_buf)
{
	fprintf(stdout, "Status: 400 %s",msg_buf);
	fprintf(stdout,"\r\n\r\n");
}

void succ_return(const char *msg_buf)
{
	fprintf(stdout, "Status: 200  %s",msg_buf);
	fprintf(stdout,"\r\n\r\n");
}



int main(int argc, char **argv, char **env)
{
	char TMP_BUF[64];
	char system_command[512];
	int upgrade_source=-1;
	memset(system_command,0,512);
	memset(TMP_BUF,0,sizeof(TMP_BUF));
	const char *filename;

	FILE *fd;

	filename=getenv("FILE_FILENAME_upload");

	if(access(StartingUpgradeFlag,0)==0){
		fd=fopen(StartingUpgradeFlag,"r");
		if(fd==NULL){
			sprintf(system_command,"rm -f %s",filename);
			system(system_command);
			goto FAIL;
		}
		memset(TMP_BUF,0,sizeof(TMP_BUF));
		ret=fread(TMP_BUF,1,sizeof(TMP_BUF),fd);
		if(ret<0){
			sprintf(system_command,"rm -f %s",filename);
			system(system_command);
			goto FAIL;
		}
		upgrade_source=atoi(TMP_BUF);
		if(upgrade_source!=1){
			sprintf(system_command,"rm -f %s",filename);
			system(system_command);
			goto FAIL;
		}else{
			/*monitor upgrade*/
		}
	}

	system("rm /raymarine/Data/firmware/* && sync");

	system("echo 1 > /raymarine/Data/Upgrade_FLAG/Start_Ugrading_Flag && sync");

	snprintf(system_command,512,"mv %s %s && sync",filename,RaymarineFirmware);

	system(system_command);

	//printf("Status: 400 Bad Request");

	//fprintf(stdout, "12345");

	return 0;

FAIL:
	fail_return(NULL);
	return 0;
}

