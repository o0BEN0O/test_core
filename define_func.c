#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JRD_PARAM_TYPE(type_id_val)           ((type_id_val) >> 16)

void main(void)
{
        unsigned int a = 0;
        unsigned int b = 0x11101101;
	memcpy(&a,&b,sizeof(unsigned int));
	printf("a %x b%x\n",a,b);
        a = JRD_PARAM_TYPE(b);
        printf("a %x b%x\n",a,b);
}
