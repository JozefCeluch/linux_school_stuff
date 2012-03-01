#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

/* Prints usage of the program */
void usage(char* progname)
{
	printf("Usage: %s \"command1\" \"command2\"\n\n", progname);
	printf("Start with two string arguments\n");
}

/* Removes spaces at the begining and end of input string
 * returns string */
char *trim_space(char* input)
{
	char* start = input;
	/*trim from left*/
	while (isspace(*start)) {
		start++;
	}

	char* ptr = start;
	char* end = start;
	/*trim from right*/
	while (*(++ptr) != '\0') {
		if (!isspace(*ptr)) { //only move end pointer if char isn't a space
			end = ptr;
		}
	}
	/*terminate trimmed string*/
	*(++end) = '\0';
	return start;
}

/* Parses input_string into array of strings according to spaces
 * returns array of strings, NULL if error
 * staus 1 if error, else 0*/
char **parse_string(char *input_string, int* status)
{
	if (input_string == NULL || status == NULL) {
		*status = 1;
		return NULL;
	}
	int input_length = strlen(input_string);

	if (input_length == 0) {
		*status = 1;
		return NULL;
	}

	int i = 0;
	input_string = trim_space(input_string);

	int space_count = 0;
	input_length = strlen(input_string);
	while (i < input_length) {
		if (isspace(input_string[i])) {
			space_count++;
		}
		i++;
	}

	/*first allocate whole array, then each string separately*/
	char **param_list;
	int word_count = space_count + 2;
	param_list = malloc(word_count * (sizeof(char*)));
	if (param_list == NULL) {
		perror("malloc() failed");
		*status = 1;
		return NULL;
	}
	for (i = 0; i < word_count - 1; i++) {
		param_list[i] = malloc(sizeof(char) * input_length);
		if (param_list[i] == NULL) {
			perror("malloc() failed");
			*status = 1;
			return NULL;
		}
		memset(param_list[i], 0, input_length);
	}
	param_list[word_count] = NULL; // parameter list terminated with NULL pointer

	/*parse input_string by spaces*/
	int j = 0;
	i = 0;
	space_count = 0;
	while (input_string[i] != '\0') {
		if (input_string[i] == ' ') {
			space_count++;
			j = 0;
		}
		if (input_string[i] != ' ') {
			param_list[space_count][j] = input_string[i];
			j++;
		}
		i++;
	}
	*status = 0;
	return param_list;
}

/* Program imitates pipe in shell using named pipe
 * Two strings are passed as arguments at start */
int main(int argc, char *argv[])
{
	int pipefd[2];
	int status;
	int errno;
	int status1 = 0;
	int status2 = 0;
	pid_t child, wpid;

	if (argc != 3) {
		fprintf(stderr, "Wrong argument amount\n");
		usage(argv[0]);
		return 1;
	}

	char **param_list1 = parse_string(argv[1], &status1);
	char *** const ptr_list1 = &param_list1;

	char **param_list2 = parse_string(argv[2], &status2);
	char *** const ptr_list2 = &param_list2;

	if (!param_list1 || !param_list2) {
		fprintf(stderr, "parse_string() failed");
		return 1;
	}

	if (mkfifo("mypipe", 0666) < 0) {
		perror("creating pipe failed");
		if (errno != EEXIST)
			return 1;
	}

	child = fork(); /*returns 0 for child*/
	if (child == -1) {
		perror("fork() failed");
		return 1;
	} else {
		if (child == 0) {
			pipefd[1] = open("mypipe", O_WRONLY);
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[1]);
			if (-1 == execvp((*ptr_list1)[0], *ptr_list1)) {
				perror("execvp() failed");
			}
			exit(EXIT_FAILURE);
		} else {
			child = fork();
			if (child == -1) {
				perror("fork() failed");
				return 1;
			} else {
				if (child == 0) {
					pipefd[0] = open("mypipe", O_RDONLY);
					dup2(pipefd[0], STDIN_FILENO);
					close(pipefd[0]);
					if (-1 == execvp((*ptr_list2)[0],
							*ptr_list2)) {
						perror("execvp() failed");
					}
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	close(pipefd[0]);
	close(pipefd[1]);

	while ((wpid = waitpid(-1, &status, 0)) > 0) {
		printf("process PID [%d] EXIT (Status: %d) \n", wpid, status);
	}

	return 0;
}
