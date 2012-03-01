#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

void *fthread (void* arg)
{
	char *ch = (char *) arg;
        printf("%d  %c\n", (pid_t) syscall(SYS_gettid), *ch);

        return 0;
}

int main()
{
	int i =0;
	char par[10];

	for (i = 0; i < 10; i++){
		par[i]='A' + i;
	}
	pthread_t thread[10];
        pid_t tid = (pid_t) syscall (SYS_gettid);
	
	for(i = 0; i < 10; i++){
                pthread_create(&thread[i], NULL, &fthread, &par[i]);
        }

	for (i = 0; i <10; i++){
		pthread_join(thread[i],NULL);
	}
	printf("Parent thread PID: %d\n", tid);
        return 0;
}
