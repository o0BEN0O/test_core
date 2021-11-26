#include <iostream>
#include <limits.h>
#include <pthread.h>
#include "error.h"
using namespace std;
int main(){
   pthread_t thread;
   size_t stacksize;
   pthread_attr_t thread_attr;
   int ret;
   pthread_attr_init(&thread_attr);
   int new_size = 20480;
   ret =  pthread_attr_getstacksize(&thread_attr,&stacksize);
   if(ret != 0){
       cout << "emError" << endl;
       return -1;
   }
 
   cout << "stacksize=" << stacksize << endl;
 
  cout << PTHREAD_STACK_MIN << endl;
   ret = pthread_attr_setstacksize(&thread_attr,new_size);
   if(ret != 0)
       return -1;
   ret =  pthread_attr_getstacksize(&thread_attr,&stacksize);
   if(ret != 0){
       cout << "emError" << endl;
       return -1;
   }
   cout << "after set stacksize=" << stacksize << endl;
   ret = pthread_attr_destroy(&thread_attr);
   if(ret != 0)
      return -1;
   return 0;
}
