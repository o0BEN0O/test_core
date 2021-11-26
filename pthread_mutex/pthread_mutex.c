#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>           //线程的取消动作发生在加锁和解锁过程中时，当发生线程2取消后而没有进行解锁时，就会出现线程1将一直阻塞


pthread_cond_t cond;
pthread_mutex_t mutex;

static int i=0;

void test_1(void)
{
	i++;
	printf("%s:[i] %d\n",__func__,i);
	//sleep(1);
}

void test(int pthread_num)
{
	//pthread_mutex_lock(&mutex);
	pthread_cond_wait(&cond, &mutex);
	i++;
	printf("[BEN]%s:[i] %d [%d]\n",__func__,i,pthread_num);
	sleep(1);
	//pthread_mutex_unlock(&mutex);
}

void* odd(void* arg)
{
	while(1){
		//pthread_cond_wait(&cond, &mutex);
		test(2);
	}
}

void* even(void* arg)
{
  while(1){
	test_1();
	pthread_cond_signal(&cond);
  }
}

void *test_func(void *arg)
{
	int a=*((int *)(arg));
	printf("start %d\n",a);
	pthread_mutex_lock(&mutex);
	//sleep(1);
	pthread_mutex_unlock(&mutex);
	printf("end %d\n",a);
}
void* pthread3(void* arg)
{
	int a=3;
        while(1){
                //pthread_cond_wait(&cond, &mutex);
                test_func(&a);
				printf("start pthread 3\n");
				pthread_mutex_lock(&mutex);
				//sleep(1);
				pthread_mutex_unlock(&mutex);
				printf("end pthread 3\n");
        }
}

void* pthread4(void* arg)
{
	int a=4;
        while(1){
                //pthread_cond_wait(&cond, &mutex);
			test_func(&a);
        }
}

void* pthread5(void* arg)
{
	int a=5;
        while(1){
                //pthread_cond_wait(&cond, &mutex);
			test_func(&a);
        }
}

void* pthread6(void* arg)
{
	int a=6;
        while(1){
                //pthread_cond_wait(&cond, &mutex);
			test_func(&a);
        }
}


int main()
{
    pthread_t t1, t2,t3,t4,t5,t6;
	//pthread_cond_init(&cond, NULL);
    //pthread_mutex_init(&mutex, NULL);
    //pthread_create(&t1, NULL, even, NULL);
    //pthread_create(&t2, NULL, odd, NULL);
	pthread_create(&t3, NULL, pthread3, NULL);
	pthread_create(&t4, NULL, pthread4, NULL);
	pthread_create(&t3, NULL, pthread5, NULL);
	pthread_create(&t4, NULL, pthread6, NULL);
    //pthread_create(&t3, NULL, even, NULL);

    //sleep(3);
    //pthread_cancel(t2);             //取消线程2，这个动作可能发生在线程2加锁之后和解锁之前

    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    //pthread_mutex_destroy(&mutex);

    return 0;
}
