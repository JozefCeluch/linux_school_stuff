#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

int main()
{
	int fd;
	fd = open("file", O_WRONLY | O_CREAT, S_IWUSR);
	printf("waiting for lock\n");
	flock(fd, LOCK_EX);
	printf("exclusive lock\n");
	getchar();
	return 0;
}
