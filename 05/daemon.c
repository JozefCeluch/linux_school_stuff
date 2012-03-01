#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>

#ifdef DBG
#define DEBUG_MSG(format,args...) syslog(LOG_INFO,"%s:%d : "format"\n",__FILE__,__LINE__,##args)
#else
#define DEBUG_MSG(format, args...)
#endif

#define LOG_LINE_SIZE 500
#define SUMMARY_SIZE 100
#define NUMBER_LENGTH 5

volatile int done = 0;
volatile int message_length = 0;
volatile int message_count = 0;
int alarm_length = 60;
char *pipe_name;

void log_summary()
{
	char summary[SUMMARY_SIZE];
	char buffer[NUMBER_LENGTH];
	memset(summary, 0, SUMMARY_SIZE);
	memset(buffer, 0, NUMBER_LENGTH);

	sprintf(buffer, "%d", message_count);
	strcat(summary, buffer);
	strcat(summary, " messages, ");
	strcat(summary, "combined length:");
	memset(buffer, 0, NUMBER_LENGTH);
	sprintf(buffer, "%d", message_length);
	strcat(summary, buffer);
	memset(buffer, 0, NUMBER_LENGTH);

	syslog(LOG_INFO, "%s: %s", pipe_name, summary);
	message_count = 0;
	message_length = 0;
}

void handler(int sig)
{
	log_summary();
	alarm(alarm_length);
}

void handle_end(int sig)
{
	remove(pipe_name);
	syslog(LOG_INFO, "Daemon stopped");
	done = 1;
}

void usage(char *progName)
{
	printf("Usage: %s \"pipe_name\" \"alarm_interval\"\n\n", progName);
	printf("Default interval: 60 seconds\n");
}

int main(int argc, char *argv[])
{
	if (argc > 3 || argv[1] == NULL) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	int current_message_length = 0;
	int lnfeed_count = 0;
	int pipefd;
	char *buf = malloc(sizeof(char*));
	if (!buf) {
		perror("malloc failed\n");
		return 1;
	}
	char *buf_help = NULL;
	char message[LOG_LINE_SIZE];
	memset(message, 0, LOG_LINE_SIZE);
	int path_size = 200;
	char *path_buf = malloc(sizeof(char) * path_size);
	if (!path_buf) {
		perror("malloc failed\n");
		return 1;
	}

	pipe_name = argv[1];
	if (argc == 3) {
		alarm_length = strtol(argv[2], (char **) NULL, 10);
		if (alarm_length == 0) {
			perror("number conversion failed");
			exit(EXIT_FAILURE);
		}
	}
	/* get current working directory */
	while (!getcwd(path_buf, path_size)) {
		if (errno == ERANGE) {
			path_size = path_size * 2;
			if (!realloc(path_buf, sizeof(char) * path_size)) {
				perror("realloc failed\n");
				return 1;
			}
		}
	}
	/* make absolute path from pipe name */
	buf_help = path_buf + strlen(path_buf);
	while (*(buf_help) != '/') {
		buf_help--;
	}
	strcat(buf_help, "/");
	strcat(buf_help, pipe_name);

	/* discard made absolute path if pipe_name is already absolute */
	if (pipe_name[0] != '/') {
		pipe_name = path_buf;
	} else {
		free(path_buf);
	}

	if (mkfifo(pipe_name, 0666) < 0) {
		perror("mkfifo() failed");
		DEBUG_MSG("mkfifo() failed: %s",strerror(errno));
		if (errno != EEXIST)
			exit(EXIT_FAILURE);
	}
	printf("Starting daemon\n");
	/* create daemon, directory changed to '\',
	 * stdi, stdo, stderr redirected to /dev/null */
	if (daemon(0, 0) < 0) {
		perror("daemon() failed");
		DEBUG_MSG("daemon() failed: %s",strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct sigaction action, end;
	sigemptyset(&end.sa_mask);
	end.sa_flags = 0;
	end.sa_handler = handle_end;
	sigaction(SIGINT, &end, NULL);
	sigaction(SIGTSTP, &end, NULL);
	sigaction(SIGKILL, &end, NULL);
	sigaction(SIGQUIT, &end, NULL);
	sigaction(SIGTERM, &end, NULL);

	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	action.sa_handler = handler;
	sigaction(SIGALRM, &action, NULL);

	alarm(alarm_length);
	while (!done) {
		while((pipefd = open(pipe_name, O_RDONLY)) < 0) {
			if (errno != EINTR){
				DEBUG_MSG("open() pipe failed: %s",strerror(errno));
				syslog(LOG_INFO, "%s: %s", pipe_name, strerror(errno));
				remove(pipe_name);
				return 1;
			}
		}

		lnfeed_count = 0;
		while (lnfeed_count < 2) {
			current_message_length = 0;
			read(pipefd, buf, 1);
			while (*buf != '\n') {
				lnfeed_count = 0;
				message_length++;
				current_message_length++;
				strcat(message, buf);
				read(pipefd, buf, 1);
			}
			lnfeed_count++;
			if (current_message_length) {
				DEBUG_MSG("length %d", current_message_length);
				message_count++;
				syslog(LOG_INFO, "%s: %s", pipe_name, message);
				memset(message, 0, LOG_LINE_SIZE);
			}
		}
		close(pipefd);
	}
	return 0;
}
