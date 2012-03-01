#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct thread_content
{
	char *file_name;
	FILE* file;
	char *phrase;
};

void *fthread(void* arg)
{
	struct thread_content *args = (struct thread_content *) arg;

	char* file_content; // whole file in one string
	int size; // size of file
	int i = 0; // counter
	int line_count = 0; // total number of lines in file
	int current_line = 0;
	int found_flag = 0;
	int phrase_length = strlen(args->phrase);

	if (fseek(args->file, 0, SEEK_END) < 0) {
		perror("fseek() failed");
		exit(EXIT_FAILURE);
	}
	if ((size = ftell(args->file)) < 0) {
		perror("ftell() failed");
		exit(EXIT_FAILURE);
	}
	rewind(args->file);

	file_content = malloc(size * sizeof(char));
	if (!file_content) {
		perror("malloc() failed");
		exit(EXIT_FAILURE);
	}
	if (!fread(file_content, 1, size, args->file)) {
		perror("fread() failed");
	}
	if (!fclose(args->file)) {
		perror("fclose() failed");
	}
	file_content[size - 1] = '\0';

	for (i = 0; i < size; i++) {
		if (file_content[i] == '\n') {
			line_count++;
		}
	}

	int *found = malloc((line_count + 1) * sizeof(int));
	if (!found) {
		perror("malloc() failed");
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < line_count + 1; i++) {
		found[i] = 0;
	}

	i = 0;
	while (file_content[i] != '\0') {
		if (file_content[i] == '\n') {
			current_line++;
		}
		if (!strncmp(&file_content[i], args->phrase, phrase_length)) {
			found_flag = 1;
			found[current_line]++;
		}
		i++;
	}
	if (found_flag) {
		for (i = 0; i < line_count + 1; i++) {
			if (found[i])
				printf("%d occurrence of \"%s\" found in \"%s\" on line no. %d\n",
						found[i], args->phrase,
						args->file_name, i + 1);
		}
	} else {
		printf("\"%s\" was not found in \"%s\"\n", args->phrase,
				args->file_name);
	}

	free(found);
	free(file_content);
	pthread_exit(&found_flag);
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("Usage: %s \"phrase\" \"files\"\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int rc;
	int i = 0;
	struct thread_content param[argc - 2];

	for (i = 0; i < argc - 2; i++) {
		param[i].file_name = argv[i + 2];
		param[i].phrase = argv[1];
		param[i].file = fopen(argv[i + 2], "r");
		if (param[i].file == NULL) {
			perror("fopen() failed");
			exit(EXIT_FAILURE);
		}
	}
	pthread_t thread[argc - 2];

	for (i = 0; i < argc - 2; i++) {
		rc = pthread_create(&thread[i], NULL, &fthread, &param[i]);
		if (rc){
			fprintf(stderr, "pthread_create() error: %s", strerror(rc));
		}
	}

	for (i = 0; i < argc - 2; i++) {
		rc = pthread_join(thread[i], NULL);
		if (rc) {
			fprintf(stderr, "pthread_join() error: %s", strerror(rc));
		}
	}
	return 0;
}
