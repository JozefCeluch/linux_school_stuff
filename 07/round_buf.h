/*! \file
 \brief A Documented file.

 Details.
 */

#ifndef ROUND_BUF_H_
#define ROUND_BUF_H_

#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FAILURE 1
#define SUCCESS 0
/*!
 * @typedef buf_t
 * @brief Structure to represent one item in buffer
 */
typedef struct buffer_item
{
	/*@{*/
	pthread_cond_t cond; /**< condition to enable writing */
	void *data; /**< data stored in item */
	pthread_mutex_t mutex; /**< mutex specific for each item */
	struct buffer_item *next; /**< pointer to the next item in buffer */
	int ord; /**< item position in buffer, just as help */
	pthread_key_t read_attr; /**< attribute to determine if item has been read by current thread */
	sem_t semaphore; /**< number of reads needed to make item writable, default 0 */
/*@}*/
} buf_t;

/*!
 * @typedef rb_t
 * @brief Structure to represent round buffer
 */
typedef struct round_buffer
{
	/*@{*/
	buf_t **buffer; /**< pointer to the begining of buffer */
	uint32_t rb_size; /**< number of items in buffer */
	uint8_t rb_reader_count;/**< number of readers */
	buf_t *write_pos; /**< writing position */
	buf_t *read_pos; /**< default reading position */
	pthread_key_t thread_pos; /**< thread specific reading position */
	void(*datafree)(void *); /**< destroyer function */
/*@}*/
} rb_t;

/*!
 * @brief Function initializes the round buffer
 *
 * Allocates all memory blocks, sets all my_rb attributes to default values.
 *
 * @param my_rb		The round buffer to initialize
 * @param datafree		The pointer to custom free function
 * @param rb_size		The number of items in the buffer
 * @param readers_count	The number of readers to read from buffer
 * @return 0 OK
 * @return 1 Error initializing buffer
 */
int rb_init(rb_t *my_rb, void(*datafree)(void *data), uint32_t rb_size,
		uint8_t readers_count);

/*!
 * @brief Function is used by writing thread to write items to buffer
 *
 * Checks if current item is writable, if not waits for signal.
 * Then frees memory pointed to by data pointer, resets the semaphore to
 * the number of readers and creates new thread specific key.
 *
 * @param my_rb 		The initialized buffer
 * @param data		The data to be written to buffer
 * @return 0 data successfully written
 */
int rb_write(rb_t *my_rb, void *data);

/*!
 * @brief Function is used by reading threads to read from the buffer
 *
 * At first run intializes thread specific possition to default reading position.
 * Copies item data pointer to output data pointer.
 * Doesn't move to the next item if it's already read by all readers.
 *
 * @param my_rb 		The initialized buffer
 * @param data			Pointer to the read item data
 * @return 0 OK, data was read
 * @return 1 Error, nothing was read
 */
int rb_read(rb_t *my_rb, void **data);

/*!
 * @brief Function destroys the buffer, frees all allocated memory, destroys all keys etc.
 * @param  my_rb 		The initialized buffer
 * @return 0 buffer correctly destroyed
 * @return 1 Error
 */
int rb_destroy(rb_t *my_rb);

#endif /* ROUND_BUF_H_ */
