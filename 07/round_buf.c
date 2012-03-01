#include "round_buf.h"

int rb_init(rb_t *my_rb, void(*datafree)(void *data), uint32_t rb_size,
		uint8_t readers_count)
{
	if (my_rb == NULL) {
		fprintf(stderr, "rb_init(): my_rb == null");
		return FAILURE;
	}

	my_rb->rb_size = rb_size;
	my_rb->rb_reader_count = readers_count;
	my_rb->buffer = malloc(sizeof(buf_t*) * rb_size);
	my_rb->datafree = datafree;

	if (pthread_key_create(&my_rb->thread_pos, NULL)) {
		perror("pthread_key_create");
		return FAILURE;
	}

	int i; //*< counter */
	/* allocate memory, set default values of each item in bufer */
	for (i = 0; i < rb_size; i++) {
		my_rb->buffer[i] = malloc(sizeof(buf_t));
		my_rb->buffer[i]->data = NULL;
		my_rb->buffer[i]->ord = i;

		if (pthread_mutex_init(&my_rb->buffer[i]->mutex, NULL)) {
			perror("pthread_mutex_init");
			rb_destroy(my_rb);
			return FAILURE;
		}
		if (pthread_key_create(&my_rb->buffer[i]->read_attr, NULL)) {
			perror("pthread_key_create");
			rb_destroy(my_rb);
			return FAILURE;
		}

		if (pthread_cond_init(&my_rb->buffer[i]->cond, NULL)) {
			perror("pthread_cond_init");
			rb_destroy(my_rb);
			return FAILURE;
		}

		/* connect each previous item to the currently allocated
		 * semaphore is initialized to write */
		if (i > 0) {
			my_rb->buffer[i - 1]->next = my_rb->buffer[i];
			if (sem_init(&my_rb->buffer[i]->semaphore, 0, 0) < 0) {
				perror("sem_init");
				rb_destroy(my_rb);
				return FAILURE;
			}
		}
		/* make buffer circular */
		if (i == rb_size - 1) {
			my_rb->buffer[i]->next = my_rb->buffer[0];
		}
	}

	my_rb->read_pos = my_rb->buffer[0];
	my_rb->write_pos = my_rb->buffer[0];

	return SUCCESS;
}
;

int rb_write(rb_t *my_rb, void *data)
{
	int val;
	buf_t *old;
	/** check value of semaphore in current buffer item, wait until signal,
	 * then check if it's open to writing */

	pthread_mutex_lock(&my_rb->write_pos->mutex);
	sem_getvalue(&my_rb->write_pos->semaphore, &val);
	while (val) {
		//		printf("sem value %d write wait mutex locked at %d writing %s\n", val, my_rb->write_pos->ord, (char *)data);
		pthread_cond_wait(&my_rb->write_pos->cond,
				&my_rb->write_pos->mutex);
		sem_getvalue(&my_rb->write_pos->semaphore, &val);
	}

	//	printf("\twriting to %d - %s\n", my_rb->write_pos->ord, (char *) data);

	/* writer might free the memory allocated by the
	 * data pointer before reader will have a chance to process it */
	free(my_rb->write_pos->data);
	my_rb->write_pos->data = data;
	old = my_rb->write_pos;

	/* reset the semaphore and thread specific attribute */
	sem_destroy(&old->semaphore);
	sem_init(&old->semaphore, 0, my_rb->rb_reader_count);
	pthread_key_delete(old->read_attr);
	pthread_key_create(&old->read_attr, NULL);

	/* move to next item */
	my_rb->write_pos = my_rb->write_pos->next;
	pthread_mutex_unlock(&old->mutex);

	return SUCCESS;
}

int rb_read(rb_t *my_rb, void **data)
{
	buf_t *old;
	int val;

	/* set thread specific pointer to default reading position */
	if (pthread_getspecific(my_rb->thread_pos) == NULL) {
		pthread_setspecific(my_rb->thread_pos, my_rb->read_pos);
	}
	old = pthread_getspecific(my_rb->thread_pos);

	pthread_mutex_lock(&old->mutex);
	sem_getvalue(&old->semaphore, &val);

	/** reader stays on first unread item and signals to writer
	 * that this item is open to writing  */
	if (val == 0) {
		pthread_cond_signal(&old->cond);
		pthread_setspecific(my_rb->thread_pos, old->next);
		pthread_mutex_unlock(&old->mutex);

		return FAILURE;
	} else if ((int) pthread_getspecific(old->read_attr)) {
		/** this item is already read by current thread,
		 * move to next item */
		pthread_setspecific(my_rb->thread_pos, old->next);
		pthread_mutex_unlock(&old->mutex);

		return FAILURE;
	}

	sem_wait(&old->semaphore);

	*data = old->data;
	//	printf("read %s from %d\n",(char *)old->data, old->ord);
	pthread_setspecific(old->read_attr, (void*) 1);

	/* last reader signals that item is open to writing */
	sem_getvalue(&old->semaphore, &val);
	if (val == 0) {
		pthread_cond_signal(&old->cond);
	}
	pthread_setspecific(my_rb->thread_pos, old->next);
	pthread_mutex_unlock(&old->mutex);

	return SUCCESS;
}

int rb_destroy(rb_t *my_rb)
{
	int i;
	for (i = 0; i < my_rb->rb_size; i++) {
		pthread_key_delete(my_rb->buffer[i]->read_attr);
		sem_destroy(&my_rb->buffer[i]->semaphore);
		pthread_mutex_destroy(&my_rb->buffer[i]->mutex);
		if (my_rb->buffer[i]->data) {
			my_rb->datafree(my_rb->buffer[i]->data);
		}
		free(my_rb->buffer[i]);
	}
	pthread_key_delete(my_rb->thread_pos);
	free(my_rb->buffer);
	return 0;
}
