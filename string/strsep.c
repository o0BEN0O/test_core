#include <stdio.h>
#include <string.h>

#define path_dnsmasq_leases "/var/lib/misc/dnsmasq.leases"


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


static char *find_dev_name(const char *ip,const char *mac)
{
	char buf[512];
	char *name=NULL,*value,*next;
	int i,j;
	FILE *fp=NULL;
	if((fp=fopen(path_dnsmasq_leases,"r"))==NULL)
	{
		fprintf(stderr,"open fail %s\n",path_dnsmasq_leases);
		return "*";
	}
	while(fgets(buf, sizeof(buf), fp) != NULL){
		value=buf;
		for(i=0;i<5;i++){
			if(i==1){
				next=strsep(&value," ");
				if(strncmp(next,mac,strlen(next))==0){
					next=strsep(&value," ");
					if(strncmp(next,ip,strlen(next))==0){
						i++;
						continue;
					}else{
						break;
					}
				}else{
					break;
				}
			}
			if(i==3){
				name=strsep(&value," ");
				fprintf(stderr,"name %s\n",name);
				goto out;
			}else{
				strsep(&value," ");
			}
		}
		//}
	}

out:
	if(name==NULL)
		return "*";
	return name;
}

void main()
{
	char *name;
	name=find_dev_name("198.18.1.18","a0:48:1c:95:ca:58");
	fprintf(stderr,"name %s\n",name);
}

