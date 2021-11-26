/*strncmpexample*/
#include<stdio.h>
#include<string.h>
#include <ctype.h>

int ipv6_global_id_check(char *ipv6_global_id)
{
	int i=0;
	int len=strlen(ipv6_global_id);
	char tmp;

	printf("ipv6_global_id [%s]\n",ipv6_global_id);
	if(len>12){
		printf("ipv6 global id [%s] length is [%d]\n",ipv6_global_id,len);
		return -1;
	}
	for(i=0;i<len;i++){
		strncpy(&tmp,ipv6_global_id+i,1);
		if(isxdigit(ipv6_global_id[i])){
			printf("detect %c\n",ipv6_global_id[i]);
			continue;
		}else if(strncmp(&tmp,":",1)==0){
			printf("detect %c\n",ipv6_global_id[i]);
			if(i==0){
				printf("detect %c\n",ipv6_global_id[i]);
				return -1;
			}
			i++;
			if(!isxdigit(ipv6_global_id[i])){
				printf("detect %c\n",ipv6_global_id[i]);
				return -1;
			}
			continue;
		}else{
			printf("ipv6 global %c is invalid\n",ipv6_global_id[i]);
			return -1;
		}
	}
	return 0;
}

int main()
{
	int ret;
	
	char ipv6_global_id[16]="1::456:7890";
	
	ipv6_global_id_check(ipv6_global_id);
	return ret;
}
