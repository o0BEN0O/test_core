#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#define lan_conf_date "/home/ben/ben/4g_router/test/lan_conf_date"

int stringFind(const char *string, const char *dest) {
	if (string == NULL || dest == NULL) return -1;

	int i = 0;
	int j = 0;
	while (string[i] != '\0') {
	    if (string[i] != dest[0]) {
	        i ++;
	        continue;
	    }
	    j = 0;
	    while (string[i+j] != '\0' && dest[j] != '\0') {
	        if (string[i+j] != dest[j]) {
	            break;
	        }
	        j ++;
	    }
	    if (dest[j] == '\0') return j;
	    i ++;
	}
	return -1;
}

int get_value_from_file(char *file_name,char *key,char *value,char *identifier)
{
	char line_buffer[64]={0};
	char get_buffer[64]={0};
	char key_buffer[64]={0};
	int line_len=0;
	int value_len=0;
	int pos=0;
	FILE *fp = fopen(file_name, "r+");
	if(identifier != NULL)
		snprintf(key_buffer,64,"%s%s",key,identifier);
	else
		snprintf(key_buffer,64,"%s",key);
	if(fp == NULL)
	{
		printf("open error");
		return -1;
	}
	while(fgets(line_buffer, 64, fp))
	{
		line_len=strlen(line_buffer);
		pos += line_len;
		if(stringFind(line_buffer,key_buffer)==strlen(key_buffer)){
			pos -= strlen(line_buffer);
			pos += strlen(key_buffer);
			value_len = (strlen(line_buffer)-strlen(key_buffer));
			fseek(fp,pos,SEEK_SET);
			fread(value,1,value_len-1,fp);/* rm \n */
			break;
		}
	}
	fclose(fp);
	return 0;
}

int set_value_to_file(char *file_name,char *key,char *value,char *identifier)
{
	char line_buffer[64]={0};
	char set_buffer[64]={0};
	char key_buffer[64]={0};
	
	char *buf1; /*save before change line*/
	char *buf2;/*save after change line*/
	char *total_buf;

	char *clean_buf;
	int line_len=0;
	int set_len=0;
	int total_len=0;
	int pos=0;
	int i=0;
	struct stat file;
	stat(file_name,&file);
	int file_size = file.st_size;
	FILE *fp = fopen(file_name, "r+");
	if(identifier != NULL){
		snprintf(key_buffer,64,"%s%s",key,identifier);
		snprintf(set_buffer,64,"%s%s%s\n",key,identifier,value);
	}
	else{
		snprintf(key_buffer,64,"%s",key);
		snprintf(set_buffer,64,"%s%s\n",key,value);
	}
	set_len=strlen(set_buffer);
	if(fp == NULL)
	{
		printf("open error");
		return -1;
	}
	while(fgets(line_buffer, 64, fp))
	{
		line_len=strlen(line_buffer);
		pos += line_len;
		if(stringFind(line_buffer,key_buffer)==strlen(key_buffer)){
			if(pos-line_len != 0){
				buf1=(char *)malloc(sizeof(char)*(pos-line_len));
				//printf("pos %d line_len%d\n",pos,line_len);
				memset(buf1,0,pos-line_len);
				fseek(fp,0,SEEK_SET);
				fread(buf1,1,pos-line_len,fp);
				//printf("1.%s\n",buf1);
			}

			buf2=(char *)malloc(sizeof(char)*(file_size-pos));
			memset(buf2,0,file_size-pos);
			fseek(fp,pos,SEEK_SET);
			fread(buf2,1,file_size-pos,fp);

			total_len=file_size-line_len+set_len;
			total_buf=(char *)malloc(sizeof(char)*(total_len+1));
			snprintf(total_buf,total_len+1,"%s%s%s",buf1,set_buffer,buf2);
			//printf("2.%s",buf2);
			fclose(fp);
			
			fp = fopen(file_name, "wt");
			fwrite(total_buf,1,total_len,fp);

			#if 0
			pos -= line_len;
			fseek(fp,pos,SEEK_SET);
			if(strlen(set_buffer)<line_len){
				
				fwrite(set_buffer,1,set_len,fp);
				//fprintf(fp,"\n");
				clean_buf = (char *)malloc(sizeof(char)*(line_len-set_len));
				for(i=0;i<line_len-set_len;i++)
					strncpy(clean_buf+i,"\0",1);
				//bzero(clean_buf,line_len);
				fseek(fp,pos+set_len,SEEK_SET);
				fwrite(clean_buf,1,line_len-set_len,fp);
				free(clean_buf);
				break;
			}
			fprintf(fp,"%s",set_buffer);
			fprintf(fp,"\n");
			break;
			#endif
		}
	}
	fclose(fp);
	return 0;
}
void main(int argc, char **argv)
{
	char key[64]={0};
	char value[64]={0};
	char ret_value[64]={0};
	strcpy(key,argv[1]);
	strcpy(value,argv[2]);
	set_value_to_file(lan_conf_date,key,value,"=");
	get_value_from_file(lan_conf_date,key,ret_value,"=");
	printf("%s %d",value,(int)strlen(value));
}
