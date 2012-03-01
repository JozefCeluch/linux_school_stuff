#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

struct job {
	struct job* next;
	int jid;
};

/* list of pending jobs */
struct job* job_queue;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t sem;

void*
thread_func (void *tid)
{
	struct job* my_job;
	while (1) {
		/* get available job */
		sem_wait(&sem);
		pthread_mutex_lock(&mutex);
		if (job_queue == NULL) {
			pthread_mutex_unlock(&mutex);
			return 0;
		} else {
			my_job = job_queue;
			/* sleep to demonstrate race condition */
			usleep(500000);

			/* remove it from the queue */
			job_queue = job_queue->next;
			/* do the job */
			printf("%d by thread %d\n", my_job->jid, *(int*)tid);
			/* clean up */
			free(my_job);

			pthread_mutex_unlock(&mutex);
		}
	}
	return NULL;
}

void* produce(void *tid)
{
	struct job* aux_job;
	struct job* new_job;
	int i = 0;
	while (1) {
		new_job = (struct job*) malloc (sizeof (struct job));

		pthread_mutex_lock(&mutex);
		aux_job = job_queue;
		job_queue = new_job;
		job_queue->jid = i;
		job_queue->next = aux_job;

		pthread_mutex_unlock(&mutex);
		sem_post(&sem);
		i++;
	}
}

int
main()
{
	pthread_t my_threads[4];
	sem_init(&sem, 0, 4);
	int tid1 = 1, tid2 = 2, tid3 = 3, tid4 = 4;

	pthread_create(&my_threads[0], NULL, thread_func, &tid1);
	pthread_create(&my_threads[1], NULL, thread_func, &tid2);
	pthread_create(&my_threads[2], NULL, thread_func, &tid3);
	pthread_create(&my_threads[3], NULL, produce, &tid4);

	pthread_join(my_threads[0], NULL);
	pthread_join(my_threads[1], NULL);
	pthread_join(my_threads[2], NULL);
	return 0;
}
