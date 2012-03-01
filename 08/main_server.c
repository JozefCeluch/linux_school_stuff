#include "server.h"

int main(int argc, char * argv[])
{

	int serv_socket; /* Socket descriptor for server */
	int clnt_socket; /* Socket descriptor for client */
	struct sockaddr_in clnt_addr; /* Client address */
	char serv_port[5];
	memset(serv_port, 0, 5);
	socklen_t clnt_length; /* Length of client address data structure */
	struct thread_data *data;
	pthread_t thread_id;
	char mssg[MAX_BUF_SIZE];
	struct user *new;
	struct user *users;	// list of users, contains ID and socket
	struct user *aux;
	int i;

	memset(mssg, 0, MAX_BUF_SIZE);
	usage();
	users = (struct user *) malloc(sizeof(struct user));
	users->next = NULL;
	users->id = 0;
	aux = users;

	char c = 0;
	set_default();
	while (c != 'q') {
		c = getchar();
		while((getchar())!='\n' && c !=EOF);
		switch (c) {
		case 'v':
			view(flights, mssg);
			printf("%s", mssg);
			break;
		case 'q':
			break;
		case 'p':
			printf("Set server port: ");
			scanf("%4s", serv_port);
			while((getchar())!='\n' && getchar() !=EOF);
			break;
		case 'c':
			clear(&flights, &users);
			break;
		case 'i':
			while (c != 'n') {
				insert();
				printf("Continue inserting? y/n:\n");
				c = getchar();
				while((getchar())!='\n' && c !=EOF);
			}
		case 'h':
		default:
			usage();
			break;
		}
	}

	if (strlen(serv_port) == 0) {
		for (i = 0; i < 4; i++) {
			serv_port[i] = '4';
		}
	}

	serv_socket = create_server_socket(serv_port);
	printf("Server started at port %s\n", serv_port);

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutex2, NULL);

	while (1) /* Run forever */
	{
		/* Set the size of the in-out parameter */
		clnt_length = sizeof(clnt_addr);

		/* Wait for a client to connect */
		if ((clnt_socket = accept(serv_socket,
				(struct sockaddr *) &clnt_addr, &clnt_length))
				< 0)
			die_error("accept() failed");
		/* clnt_socket is connected to a client! */

		if ((data = (struct thread_data *) malloc(
				sizeof(struct thread_data))) == NULL)
			die_error("malloc() failed");
		data->clnt_socket = clnt_socket;

		aux = users;
		/* no user ID checking, new ID for every connection */
		if (aux->id == 0) {
			aux->id = 1;
			data->clnt_id = 1;
		} else {
			new = (struct user *) malloc(sizeof(struct user));
			new->next = NULL;

			while (aux->next != NULL) {
				aux = aux->next;
			}
			aux->next = new;
			new->id = 1 + aux->id;

			data->clnt_id = new->id;
		}

		printf("Handling client %s, socket: %d, id: %d\n", inet_ntoa(clnt_addr.sin_addr), data->clnt_socket, data->clnt_id);
		if (pthread_create(&thread_id, NULL, thread_func, (void *) data)
				!= 0)
			die_error("pthread_create() failed");
		printf("with thread %ld\n", (long int) thread_id);

	}

	return 0;
}
