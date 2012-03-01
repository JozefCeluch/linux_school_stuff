/*
 * server.h
 *
 *  Created on: Nov 27, 2011
 *      Author: dodo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#ifndef SERVER_H_
#define SERVER_H_

#define NAME_LENGTH 22
#define BACKLOG 10
#define MAX_BUF_SIZE 1024
#define LINE_SIZE 80

#ifdef DEBUG
#define DBG(format, args...) printf("[%d, %s()] "format, __LINE__, __FUNCTION__,##args)

#else
#define DBG(format, args...)
#endif

typedef struct flight_struct
{
	char from[NAME_LENGTH];
	char to[NAME_LENGTH];
	int id;
	int seat_count;
	int free_seats;
	struct flight_struct *next;
	int* seats; // default is array of '0', every client sets his socket number to his booked seat
} flight_t;

struct thread_data
{
	int clnt_socket;
	char *clnt_name;
	int clnt_id;
};

struct user
{
	char username[20];
	int id;
	struct user *next;
};

pthread_mutex_t mutex, mutex2;

flight_t *flights;	// list of flights in database

/*
 * Creates and inserts new flight into flights list
 */
int insert();

/*
 * Clears flight list
 */
void clear(flight_t **flight_list, struct user **users);

/*
 * Prints usage
 */
void usage();

/*
 * Creates default flight list, for debugging purposes
 */
void set_default();

/*
 * Prints complete flight list to buffer
 */
void view(flight_t *flight_list, char *buffer);

void die_error(char *err_mssg);

/*
 * Sends response to client
 */
void send_response(int sock, char *resp);

/*
 * Receives message from client
 */
int receive_message(int sock, char *buffer);

/*
 * Processes command from client
 */
void client_command(int sock, int clnt_id, char c);

/*
 * Books seat on a flight for client with clnt_id
 */
void booking(int sock, int clnt_id);

/*
 * Client handler
 */
void handle_client(int clnt_socket, int clnt_id);

int create_server_socket(char *port);

/*
 * Prints booked flights of client with clnt_id to buff
 */
void view_booked(int sock, int clnt_id);

void *thread_func(void *data);

#endif /* SERVER_H_ */
