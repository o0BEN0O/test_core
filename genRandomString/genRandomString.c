#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
//产生长度为length的随机字符串
int genRandomString2(int length,char* ouput)
{
    int flag, i;
    struct timeval tpstart;
    gettimeofday(&tpstart,NULL);
    srand(tpstart.tv_usec);
    for (i = 0; i < length; i++)
    {
        flag = rand() % 3;
        switch (flag)
        {
        case 0:
            ouput[i] = 'A' + rand() % 26;
            break;
        case 1:
            ouput[i] = 'a' + rand() % 26;
            break;
        case 2:
            ouput[i] = '0' + rand() % 10;
            break;
        default:
            ouput[i] = 'x';
            break;
        }
    }
    return 0;
}

void main(void)
{
	char password[12];
	memset(password,0,12);
	genRandomString2(12,password);
	printf("%s\n",password);
}
