#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

/* Prints usage of the program */
void usage(char* progname)
{
	printf("Usage: %s \"command1\" \"command2\"\n\n", progname);
	printf("Start with two string arguments\n");
}

/* Program imitates pipe in shell using unnamed pipe
 * Two strings are passed as arguments at start */
int main(int argc, char *argv[])
{
	int pipefd[2];
	int status1 = 0;
	int status2 = 0;
	pid_t child1, child2;

	if (argc != 3) {
		fprintf(stderr, "Wrong argument amount\n");
		usage(argv[0]);
		return 1;
	}

	if (pipe(pipefd) == -1) {
		perror("creating pipe failed");
		return 1;
	}

	child1 = fork(); /*returns 0 for child*/
	if (child1 == -1) {
		perror("fork() failed");
		return 1;
	}

	if (child1 == 0) {
		close(pipefd[0]);
		close(STDOUT_FILENO);
		dup2(pipefd[1], STDOUT_FILENO);
		execlp("/bin/sh", "sh", "-c", argv[1], (char *) 0);
		perror("execlp() failed");
		exit(EXIT_FAILURE);
	} else {
		child2 = fork();
		if (child2 == -1) {
			perror("fork() failed");
			return 1;
		}
		if (child2 == 0) {
			close(pipefd[1]);
			close(STDIN_FILENO);
			dup2(pipefd[0], STDIN_FILENO);
			execlp("/bin/sh", "sh", "-c", argv[2], (char *) 0);
			perror("execvp() failed");
			exit(EXIT_FAILURE);
		}
	}
	close(pipefd[0]);
	close(pipefd[1]);

	waitpid(child1, &status1, 0);
	waitpid(child2, &status2, 0);
	fprintf(stderr, "process PID [%d] EXIT (Status: %d) \n", child1,
			status1);
	fprintf(stderr, "process PID [%d] EXIT (Status: %d) \n", child2,
			status2);

	return 0;
}
