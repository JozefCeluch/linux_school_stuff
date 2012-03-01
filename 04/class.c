#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	pid_t pid;
	pid = fork();
	pid_t child = 0;
	if (pid != -1){
		if (pid != 0 ){
			printf("I'm parent %d\n", getpid());
		} else {
			printf("I'm child %d\n", child = getpid());
			execl("/bin/ps","/bin/ps","-a", (char *)NULL);
		}
	} else {
		return 1;
	}
	int status;
	printf("Parent PID %d, child PID %d\n", getpid(), waitpid(-1, NULL, 0));

	return 0;
}
