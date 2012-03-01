#include <stdio.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>

int volatile done = 0;

void handler(int a){
	done = 1;
}

int main (int argc, char *argv[])
{

	if (argc != 2) {
		printf("Usage: %s string\n", argv[0]);
		return 1;
	}
	
	int fd;
	int length = strlen(argv[1]);
	char *mm_addr;
	struct sigaction action;
	
	if ((fd = open("./mapped_file", O_CREAT | O_RDWR,
			S_IRUSR | S_IWUSR)) == -1) {
		perror("open failed");
	}
	
	lseek(fd, length+1, SEEK_SET);
	write(fd, "", 1);
	lseek(fd, 0, SEEK_SET);

	
	mm_addr = (char *)mmap(NULL, length, PROT_WRITE, MAP_SHARED, fd, 0);
	if (mm_addr == MAP_FAILED) {
		perror("mmap failed");
	}
	
	strncpy(mm_addr, argv[1], length);
	mm_addr[length] = '\0';
	msync(mm_addr, length, MS_SYNC);
	
	
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	action.sa_handler = handler;
	sigaction(SIGTSTP, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT, &action, NULL);
		
	while(!done){
		printf("%s\n", mm_addr);
		sleep(1);
	}
	
	munmap(mm_addr, length);
	close(fd);
	return 0;
}
