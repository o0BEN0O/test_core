//使用strtok()函数:
 
 
#include<stdio.h>
#include<string.h>
int main(void)
{
    char buf[]="V1.11 2020-12-23 17:10";
    printf("%d\n",strlen(buf));
    float ipq_version;
    int ipq_version_int;
    sscanf(buf,"V%f",&ipq_version);
    ipq_version_int = (int)(ipq_version*100);
    printf("%d\n",ipq_version_int);
//    return 0;

   char buf_1[]="V0.11 2021-01-04 02:00";
   char part_0[5];
   char part_1[10];
   char part_2[5];
   int part1,part2,part3,part4;
   sscanf(buf_1,"%s %s %s",part_0,part_1,part_2);
   sscanf(part_1,"%d-%d-%d",&part1,&part2,&part3);
   sscanf(part_2,"%d:",&part4);
   printf("%d %02d %02d %02d\n",part1,part2,part3,part4);

  return 0;

}
