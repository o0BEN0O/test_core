#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>           //线程的取消动作发生在加锁和解锁过程中时，当发生线程2取消后而没有进行解锁时，就会出现线程1将一直阻塞



#define JRD_LOCK(ptr) \
do { \
     pthread_mutex_lock((&(ptr)->mutex)); \
}while(0)

#define JRD_UNLOCK(ptr)  \
do { \
     pthread_mutex_unlock((&(ptr)->mutex)); \
}while(0)


#define JRD_OS_SIGNAL_WAIT(ptr)  \
  do { \
    JRD_LOCK(ptr);\
    pthread_cond_wait(&(ptr)->cond, &(ptr)->mutex); \
    JRD_UNLOCK(ptr);; \
  } while(0)

#define JRD_OS_SIGNAL_SET(ptr)  \
  do { \
    JRD_LOCK(ptr);\
    (ptr)->sig_set = 1; \
    pthread_cond_signal(&(ptr)->cond); \
    JRD_UNLOCK(ptr);\
  } while(0)


#define JRD_OS_SIGNAL_INIT(ptr) \
  do { \
    (ptr)->sig_set = 0; \
    (ptr)->timed_out = 0; \
    pthread_cond_init(&(ptr)->cond, NULL); \
    pthread_mutex_init(&(ptr)->mutex, NULL); \
  } while(0)


typedef struct {
    int sig_set;
    int timed_out;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} os_signal_type;


static int i=0;


static os_signal_type os_signal;


void test_1(void)
{
	i++;
	printf("%s:[i] %d\n",__func__,i);
	//sleep(1);
}

void* odd(void* arg)
{
	while(1){
		//pthread_cond_wait(&cond, &mutex);
		//test(2);
	}
}

void* even(void* arg)
{
  while(1){
	test_1();
	//pthread_cond_signal(&cond);
  }
}

void* wait(void* arg)
{
	static int j=0;
	while(1){
		sleep(1);
		JRD_OS_SIGNAL_WAIT(&os_signal);
		j++;
		printf("wait %d\n",j);
	}
}

void* signal(void* arg)
{
	while(1){
		JRD_OS_SIGNAL_SET(&os_signal);
	}
}

void* pthread3(void* arg)
{
	int a=3;
        while(1){
                //pthread_cond_wait(&cond, &mutex);
             //   test_func(&a);
				printf("start pthread 3\n");
			//	pthread_mutex_lock(&mutex);
			//	//sleep(1);
			//	pthread_mutex_unlock(&mutex);
			//	printf("end pthread 3\n");
        }
}

void* pthread4(void* arg)
{
	int a=4;
        while(1){
                //pthread_cond_wait(&cond, &mutex);
			//test_func(&a);
        }
}

void* pthread5(void* arg)
{
	int a=5;
        while(1){
                //pthread_cond_wait(&cond, &mutex);
			//test_func(&a);
        }
}

void* pthread6(void* arg)
{
	int a=6;
        while(1){
                //pthread_cond_wait(&cond, &mutex);
			//test_func(&a);
        }
}


int main()
{
    pthread_t t1, t2,t3,t4,t5,t6;

	JRD_OS_SIGNAL_INIT(&os_signal);
	//pthread_cond_init(&cond, NULL);
    //pthread_mutex_init(&mutex, NULL);
    //pthread_create(&t1, NULL, even, NULL);
    //pthread_create(&t2, NULL, odd, NULL);
	//pthread_create(&t3, NULL, pthread3, NULL);
	//pthread_create(&t4, NULL, pthread4, NULL);
	//pthread_create(&t3, NULL, pthread5, NULL);
	//pthread_create(&t4, NULL, pthread6, NULL);
    //pthread_create(&t3, NULL, even, NULL);
    pthread_create(&t1, NULL, wait, NULL);
	pthread_create(&t2, NULL, signal, NULL);

    //sleep(3);
    //pthread_cancel(t2);             //取消线程2，这个动作可能发生在线程2加锁之后和解锁之前

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    //pthread_mutex_destroy(&mutex);

    return 0;
}
