#include "server.h"

void usage()
{
	printf("Press:\n");
	printf("\ti\tTo insert flights into database\n");
	printf("\tv\tTo view database\n");
	printf("\tc\tTo clear database\n");
	printf("\tp\tTo change port number\n");
	printf("\th\tTo show this help\n");
	printf("\tq\tTo finish editing and start server\n");
}

int insert()
{
	flight_t *new;
	flight_t *aux = flights;
	char buff[NAME_LENGTH];

	if (flights && flights->id != 0) {
		DBG("flights != NULL\n");
		new = (flight_t *) malloc(sizeof(flight_t));
		memset(new->from, 0, NAME_LENGTH);
		memset(new->to, 0, NAME_LENGTH);
		new->next = NULL;
		new->seat_count = 0;

		while (aux->next != NULL) {
			aux = aux->next;
		}
		aux->next = new;
		new->id = 1 + aux->id;

	} else {
		DBG("flights == NULL\n");
		flights = (flight_t*) malloc(sizeof(flight_t));
		flights->next = NULL;
		flights->id = 0;
		flights->seat_count = 0;
		new = flights;
		memset(new->from, 0, NAME_LENGTH);
		memset(new->to, 0, NAME_LENGTH);
		new->id = 1;
	}
	printf("Write flight departure city (max 20 characters):\n");
	fgets(new->from, sizeof(new->from), stdin);

	while (!isalnum(new->from[0])) {
		printf("Name can't be empty!\n");
		fgets(new->from, sizeof(new->from), stdin);
	}
	new->from[strlen(new->from) - 1] = '\0';

	printf("Write flight destination (max 20 characters):\n");
	fgets(new->to, sizeof(new->to), stdin);
	while (!isalnum(new->to[0])) {
		printf("Name can't be empty!\n");
		fgets(new->to, sizeof(new->to), stdin);
	}
	new->to[strlen(new->to) - 1] = '\0';

	printf("Write number of available seats:\n");
	fgets(buff, sizeof(buff), stdin);
	while (!sscanf(buff, "%d", &new->seat_count) || new->seat_count == 0) {
		printf("Seat count can't be 0\n");
		fgets(buff, sizeof(buff), stdin);
	}
	new->free_seats = new->seat_count;
	new->seats = malloc(sizeof(int) * new->seat_count);
	memset(new->seats, 0, new->seat_count * sizeof(int));

	return 0;
}

void set_default()
{
	int i;
	flight_t *new;
	flight_t *aux;
	char num[2];
	num[1] = '\0';
	for (i = 0; i < 5; i++) {
		if (flights && flights->id != 0) {
			new = (flight_t *) malloc(sizeof(flight_t));
			memset(new->from, 0, NAME_LENGTH);
			memset(new->to, 0, NAME_LENGTH);
			new->next = NULL;
			new->seat_count = 0;
			aux = flights;
			while (aux->next != NULL) {
				aux = aux->next;
			}
			aux->next = new;
			new->id = 1 + aux->id;

		} else {
			flights = (flight_t*) malloc(sizeof(flight_t));
			flights->next = NULL;
			flights->id = 0;
			flights->seat_count = 0;
			new = flights;
			memset(new->from, 0, NAME_LENGTH);
			memset(new->to, 0, NAME_LENGTH);
			new->id = 1;
		}
		num[0] = '1' + i;
		memcpy(new->from, "DEP", strlen("DEP"));
		strcat(new->from, num);
		memcpy(new->to, "DEST", strlen("DEST"));
		strcat(new->to, num);
		new->seat_count = 10 + rand() % 20;
		new->free_seats = new->seat_count;
		new->seats = malloc(sizeof(int) * new->seat_count);
		memset(new->seats, 0, new->seat_count * sizeof(int));
//		int j;
//		for (j = 0; j < new->seat_count; j++) {
//			printf("%d ", new->seats[j]);
//		}
//		printf("\n");
	}
}

void view(flight_t *flight_list, char *buffer)
{
	DBG("print view\n");
	char *str;
	char *copy;
	memset(buffer, 0, MAX_BUF_SIZE);
	if (!flight_list || flight_list->id == 0) {
		str = "List is empty\n";
		memcpy(buffer, str, strlen(str));
	} else {
		str = "ID     FROM                TO                    AVAILABLE SEATS\n";
		sprintf(buffer, "%s", str);
		while (flight_list != NULL) {
			copy = malloc(sizeof(str) * strlen(str));
			sprintf(copy, "%d      %-20s  %-20s  %d/%d\n",
					flight_list->id, flight_list->from,
					flight_list->to, flight_list->free_seats,
					flight_list->seat_count);
			strcat(buffer, copy);
			free(copy);
			flight_list = flight_list->next;
		}
	}
}

void clear(flight_t **flight_list, struct user **users)
{
	flight_t *cur_flight;
	struct user *cur_user;

	while (*flight_list != NULL) {
		cur_flight = *flight_list;
		*flight_list = (*flight_list)->next;
		cur_flight->id = 0;
		free(cur_flight->seats);
		free(cur_flight);
		cur_flight = NULL;
	}

	while (*users != NULL) {
		cur_user = *users;
		*users = (*users)->next;
		free(cur_user);
	}

	*flight_list = NULL;
	*users = NULL;
	printf("Flight list erased\n");
}

void die_error(char *err_mssg)
{
	perror(err_mssg);
	exit(1);
}

void send_response(int sock, char *resp)
{
	int length = strlen(resp);
	if (send(sock, resp, length, 0) != length) {
		die_error("send() failed");
	}
}

void client_command(int sock, int clnt_id, char c)
{
	DBG("get command from %d on socket %d\n", clnt_id, sock);
	char mssg[MAX_BUF_SIZE];
	switch (c) {
	case 'v':
		view(flights, mssg);
		send_response(sock, mssg);
		break;
	case 'b':
		booking(sock, clnt_id);
		break;
	case 'm':
		pthread_mutex_lock(&mutex2); // NOT FINAL, maybe two mutexes would be better
		DBG("mutex2 locked by %d\n", clnt_id);
		view_booked(sock, clnt_id);
		DBG("show flights, socket: %d\n", sock);
		pthread_mutex_unlock(&mutex2);
		DBG("mutex2 unlocked %d\n", clnt_id);
		break;
	default:
		break;
	}
}

int receive_message(int sock, char *buffer)
{
	int rcvd_mssg_size;
	if ((rcvd_mssg_size = recv(sock, buffer, MAX_BUF_SIZE-1, 0)) < 0) {
		die_error("recv() failed");
	}
	return rcvd_mssg_size;
}

int print_flight(int id, char *buff)
{
	flight_t *aux = flights;
	while (aux != NULL && aux->id != id) {
		aux = aux->next;
	}
	if (aux != NULL) {
		sprintf(buff, "%d      %-20s  %-20s", aux->id, aux->from,
				aux->to);
		return 0;
	}
	return 1; // nothing was found
}

void view_booked(int sock, int clnt_id)
{
	DBG("client ID %d\n", clnt_id);
	int count = 0;
	char loc_buff[LINE_SIZE];
	char buffer[LINE_SIZE];
	int i;
	flight_t *current_flight = flights;
	memset(buffer, 0, LINE_SIZE);

	strcpy(loc_buff, "Your booked flights:\n");
	send_response(sock, loc_buff);
	while (current_flight != NULL) {
		for (i = 0; i < current_flight->seat_count; i++) {
			if (current_flight->seats[i] == clnt_id) {
				//				print_flight(aux->id, loc_buff);
				memset(loc_buff, 0, LINE_SIZE);
				sprintf(loc_buff, "%d      %-20s  %-20s",
						current_flight->id, current_flight->from, current_flight->to);
				strcpy(buffer, loc_buff);
				memset(loc_buff, 0, LINE_SIZE);
				sprintf(loc_buff, "\tseat number: %d\n", 1 + i);
				strcat(buffer, loc_buff);
				count++;
			}
			send_response(sock, buffer);
			memset(buffer, 0, LINE_SIZE);
		}
		current_flight = current_flight->next;
	}
	if (count == 0) {
		strcpy(buffer, "No booked flights.\n");
		send_response(sock, buffer);
	}
}

void booking(int sock, int clnt_id)
{
	DBG("client id %d\n", clnt_id);
	char buff[MAX_BUF_SIZE];
	char help_buff[20];
	int id = 0;
	int seat_num = 0;

	int i = 0, help_pos = 0;
	flight_t *aux_flight = flights;

	memset(help_buff, 0, 20);
	view(flights, buff);
	send_response(sock, buff);

	memset(buff, 0, MAX_BUF_SIZE);
	receive_message(sock, buff);

	/* extract numbers from buffer and convert to int */
	while (buff[i] != '-') {
		help_buff[i] = buff[i];
		i++;
	}
	i++;
	help_pos = i;
	id = atoi(help_buff);
	memset(help_buff, 0, 20);
	
	while (buff[i] != '\0') {
		help_buff[i - help_pos] = buff[i];
		i++;
	}
	seat_num = atoi(help_buff);

	while (aux_flight != NULL && aux_flight->id != id) {
		aux_flight = aux_flight->next;
	}

	if (aux_flight != NULL) {
		if (seat_num <= aux_flight->seat_count) {

			pthread_mutex_lock(&mutex);
			DBG("mutex locked by %d\n", clnt_id);
			if (aux_flight->seats[seat_num - 1] == 0) {
				aux_flight->seats[seat_num - 1] = clnt_id;
				aux_flight->free_seats--;
				int i;
				for (i = 0; i < aux_flight->seat_count; i++) {
					printf("%d ", aux_flight->seats[i]);
				}
				printf(" flight ID: %d\n", aux_flight->id);
				send_response(sock, "Operation successfull\n");
			} else {
				send_response(sock, "Seat is already taken\n");
			}
			pthread_mutex_unlock(&mutex);
			DBG("mutex unlocked by %d\n", clnt_id);
		} else {
			send_response(sock,
					"Seat with this number doesn't exist on this flight\n");
		}
	} else {
		send_response(sock, "Flight with this ID desn't exist\n");
	}

}

void handle_client(int clnt_socket, int clnt_id)
{
	char echo_buffer[MAX_BUF_SIZE]; /* Buffer for echo string */
	int rcvd_mssg_size; /* Size of received message */

	rcvd_mssg_size = receive_message(clnt_socket, echo_buffer);
	/* Send received string and receive again until end of transmission */
	while (rcvd_mssg_size > 0) /* zero indicates end of transmission */
	{
		if (rcvd_mssg_size == 1) {
			DBG("socket %d\n", clnt_socket);
			client_command(clnt_socket, clnt_id, echo_buffer[0]);
		}
		rcvd_mssg_size = receive_message(clnt_socket, echo_buffer);
	}
	DBG("client %d disconnected\n", clnt_id);
	close(clnt_socket); /* Close client socket */
}

int create_server_socket(char *port)
{
	int serv_socket; /* socket to create */
	struct addrinfo serv_addr, *res; /* Local address */

	/* Create socket for incoming connections */
	if ((serv_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		die_error("socket() failed");

	/* Construct local address structure */
	memset(&serv_addr, 0, sizeof serv_addr);
	serv_addr.ai_family = AF_UNSPEC; // use IPv4 or IPv6
	serv_addr.ai_socktype = SOCK_STREAM;
	serv_addr.ai_flags = AI_PASSIVE; // use local IP address

	getaddrinfo(NULL, port, &serv_addr, &res);

	/* Bind to the local address */
	if (bind(serv_socket, res->ai_addr, res->ai_addrlen) < 0)
		die_error("bind() failed");

	/* Mark the socket so it will listen for incoming connections */
	if (listen(serv_socket, BACKLOG) < 0)
		die_error("listen() failed");

	freeaddrinfo(res);
	return serv_socket;
}

void *thread_func(void *data)
{
	int clnt_socket;
	int clnt_id;

	/* resources are automatically released without the need for join */
	pthread_detach(pthread_self());

	/* extract socket file descriptor from argument */
	clnt_socket = ((struct thread_data *) data)->clnt_socket;
	clnt_id = ((struct thread_data *) data)->clnt_id;

	free(data); /* deallocate memory for argument */

	handle_client(clnt_socket, clnt_id);
	return NULL;
}

