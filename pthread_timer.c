#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef void (*timer_callback)();

/*定时器参数*/
typedef struct
{
	u32 interval_time; 		/* 时间间隔，单位秒 */
	timer_callback func; 			/* 处理函数 */
}timer_para;

int i = 1;

struct timeval tvafter,tvpre;
struct timezone tz;


/*定时器处理线程*/
void start_timer(void *argv)
{
	struct itimerval tick;
	timer_para * time_val;

	time_val = (timer_para *)argv;
	pthread_detach(pthread_self());
		
	printf("AppStartTimer start!\n");
	signal(SIGALRM, time_val->func);
	memset(&tick, 0, sizeof(tick));
	
	//Timeout to run first time
	tick.it_value.tv_sec = 0;
	tick.it_value.tv_usec = time_val->interval_time;
	
	//After first, the Interval time for clock
	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = time_val->interval_time;
	
	if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
	{
		perror("Set timer failed!\n");
	}
	
	while(1)
	{
		pause();
	}

	printf("AppStartTimer exit!\n");
}


int timercreate(timer_para *time_val)
{
	int ret = -1;
	pthread_t timerid;
	
	ret=pthread_create(&timerid,NULL,(void *)start_timer,time_val); 
	if(0 != ret)
	{
		printf("create AppProcessTimer failed!ret=%d err=%s\n",ret, strerror(ret));
	}

	return ret;
}

void timer_callback_func()
{
	if(i%2 == 0){
		gettimeofday (&tvpre , &tz);
		printf("花费时间:%ld ms\n",(tvpre.tv_sec-tvafter.tv_sec)*1000+(tvpre.tv_usec-tvafter.tv_usec)/1000);
	}
	if(i%2 != 0)
		gettimeofday (&tvafter , &tz);
	i++;
}

int main()
{
	timer_para time_val = {0};

	printf("main thread-----\n");

	//timer thread
	time_val.interval_time = 10000;
	time_val.func = timer_callback_func;
	timercreate(&time_val);
	
	 //主进程进入循环休眠中,数据处理主要在回调函数
	while(1)
	{
		sleep(9999);
	}

	return 0;
}

