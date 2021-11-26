#include <stdio.h>
#include <string.h>
 

int read_file(char *file,char *buf,int size)
{
	FILE *fp=NULL;
	int ret;

	//itoa(s,buf,10);
	fp = fopen(file , "rt" );
	if(fp==NULL){
		//JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"open %s failed!\n",file);
		perror("fread:");
		fclose(fp);
		return -1;
	}

	ret = fread(buf,1,size,fp);/*fread successful: ret == size*/
	if(ret!=size){
		//JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"read %s failed!\n",file);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

int main()
{
char buf[128];
read_file("./file.txt",buf,128);
return 0;
#if 0
   FILE *fp;
   char c[] = "This is runoob";
   char buffer[20];
   int ret;
 
   /* 打开文件用于读写 */
   fp = fopen("./file.txt", "w+");
 
   /* 写入数据到文件 */
   fwrite(c, strlen(c), 1, fp);
 
   /* 查找文件的开头 */
   fseek(fp, 0, SEEK_SET);
 
   /* 读取并显示数据 */
   ret=fread(buffer, 1,20, fp);
   printf("%d\n", ret);
   fclose(fp);
   
   return(0);
#endif
}
