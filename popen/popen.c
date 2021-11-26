#include<stdio.h> 
#include<unistd.h> 
#include<string.h>   
int main() 
{ 
    FILE *fp=NULL; 
    FILE *fh=NULL; 
    char buff[4096]={0};   
   memset(buff,0,sizeof(buff)); 
   fp=popen("core_api -r \"GetHubInfo\" -p '{\"jsonrpc\":\"2.0\",\"method\":\"GetHubInfo\" ,\"params\":{},\"id\":\"9.1\"}'","r");//将命令ls-l 同过管道读到fp 
   //fh=fopen("shell.c","w+");// 创建一个可写的文件 
   //fread(buff,1,sizeof(buff),fp);//将fp的数据流读到buff中
   //fwrite(buff,1,127,fh);//将buff的数据写入fh指向的文件中   
   while(fgets(buff,sizeof(buff),fp)!=NULL)
	 printf("%s",buff);
   pclose(fp); 
   //printf("%s",buff);
   //fclose(fh);   
   return 0;   
}        
