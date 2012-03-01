#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

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
	int pgsize = getpagesize();
	int shm_id;
	void *shared_mem;
	struct shmid_ds shm;
	struct sigaction action;
	
	key_t shared_key = ftok(argv[0], 1); // symlink won't work
	shm_id = shmget(shared_key, pgsize,IPC_CREAT | S_IRUSR | S_IWUSR);
	shared_mem = shmat(shm_id, 0, 0);
	
	strncpy((char *)shared_mem, argv[1], pgsize-1);
	
	if (strlen(argv[1]) >= pgsize-1){
		((char *)shared_mem)[pgsize - 1] = '\0';
	} else{
		((char *)shared_mem)[strlen(argv[1])] = '\0';
	}

	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	action.sa_handler = handler;
	sigaction(SIGTSTP, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT, &action, NULL);
		
	while(!done){
		printf("%s\n", (char*)shared_mem);
		sleep(1);
	}
	
	// get info
	if (shmctl(shm_id, IPC_STAT, &shm) == -1) {
		perror("shmctl ERROR");
	}
	// last process marks mem segment to be removed
	if (shm.shm_nattch == 1) {
		if (shmctl(shm_id, IPC_RMID, &shm) == -1) {
			perror("shmctl ERROR");
		}
	}
	shmdt(shared_mem);

	return 0;
}
