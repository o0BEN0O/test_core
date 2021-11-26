#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h> 
 
void *f()
{
	int status;
	printf("begin: pid: %d, tid:%ld, self: %ld\n", getpid(), (long int)syscall(__NR_gettid), pthread_self());
	int ret = fork();
	if(ret == 0){
		printf("[child] pid: %d, tid:%ld, self: %ld\n", getpid(), (long int)syscall(__NR_gettid), pthread_self());
	}else if(ret > 0){
		printf("[parent] pid: %d, tid:%ld, self: %ld\n", getpid(), (long int)syscall(__NR_gettid), pthread_self());
		waitpid(-1, &status, 0);
	}
}
 
int main()
{
	
	int i = 0;
	pthread_t pth[1]; 
	while(i++<1){
		pthread_create(&pth[i], NULL, f, NULL);
		sleep(1);
	}
	pause();
}
