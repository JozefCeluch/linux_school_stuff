#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_BUF_SIZE 1024   /* Size of receive buffer */

#ifdef DEBUG
#define DBG(format, args...) printf("[%s:%d, %s()]"format,__FILE__, __LINE__, __FUNCTION__,##args)

#else
#define DBG(format, args...)
#endif
void die_error(char *errorMessage)
{
	perror(errorMessage);
	exit(1);
}

void usage()
{
	printf("Press:\n");
	printf("\tv\tTo view database\n");
	printf("\tb\tTo book a flight\n");
	printf("\tm\tTo view booked flights\n");
	printf("\th\tTo show this help\n");
	printf("\tq\tTo quit\n");
}

int receive_message(int sock, char *buffer)
{
	int rcvd_mssg_size = 0;
	memset(buffer, 0, MAX_BUF_SIZE);
//	DBG("rcvd before\n");
	if ((rcvd_mssg_size = recv(sock, buffer, MAX_BUF_SIZE-1, 0))
			< 0) {
		if(errno != EAGAIN)
			die_error("recv() failed");
	}
//	DBG("rcvd = %d\n", rcvd_mssg_size);
	return rcvd_mssg_size;
}

void send_response(int sock, char *resp)
{
	int length = strlen(resp);
//	printf("%s %d\n",resp ,length);
	if (send(sock, resp, length, 0) != length) {
		die_error("send() failed");
	}
}

void book_flight(int sock, char *init)
{
	send_response(sock, init);
	char buff[MAX_BUF_SIZE];
	char id[20];
	char seat_num[20];
	int num = 0;
	memset(buff, 0, MAX_BUF_SIZE);
	receive_message(sock, buff);
	printf("%s\n", buff);

	printf("Write an ID of the flight you want to book:\n");
	fgets(id, sizeof(id), stdin);
	while (!sscanf(id, "%d", &num) || num == 0){
		printf("Flight ID must be valid number!\n");
		fgets(id, sizeof(id), stdin);
	}

	printf("Write the seat number:\n");
	fgets(seat_num, sizeof(seat_num), stdin);
	while (!sscanf(seat_num, "%d", &num) || num == 0){
		printf("Seat number must be valid!\n");
		fgets(seat_num, sizeof(seat_num), stdin);
	}
	memset(buff, 0, MAX_BUF_SIZE);
	strcat(buff, id);
	strcat(buff,"-");
	strcat(buff, seat_num);
	send_response(sock, buff);
	memset(buff, 0, MAX_BUF_SIZE);
	receive_message(sock, buff);
	printf("%s\n", buff);

//	while (buff[0] != 1){
//		receive_message(sock, buff);
//		printf("%s", buff+1);
//		scanf("%20s", buff);
//		send_response(sock, buff);
//	}
}


int main(int argc, char *argv[])
{

	/* ask for username, if it exists fail, no passwd necessary */

	int sock; /* Socket descriptor */
	struct addrinfo serv_addr, *res; /* Echo server address */
	char *serv_port; /* Echo server port */

	char *serv_ip; /* Server IP address (dotted quad) */
	char rcv_buffer[MAX_BUF_SIZE]; /* Buffer for echo string */

	char c = 0;
	char username[20];
	memset(username, 0, 20);
	char param[2];
	memset(param, 0, 2);

	int rc;

	if (argc < 3) /* Test for correct number of arguments */
	{
		fprintf(stderr, "Usage: %s <Server IP> <Echo Port>\n", argv[0]);
		exit(1);
	}

	usage();

	serv_ip = argv[1]; /* First arg: server IP address (dotted quad) */
//	send_strin = argv[2]; /* Second arg: string to echo */

	if (argc == 3)
		serv_port = argv[2]; /* Use given port, if any */
	else
		serv_port = "4444"; /* default port */

	/* Create a reliable, stream socket using TCP */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		die_error("socket() failed");

	/* timeout used to wait for recv */
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof tv)) {
		die_error("setsockopt");
	}

	memset(&serv_addr, 0, sizeof serv_addr);
	serv_addr.ai_family = AF_UNSPEC;  // use IPv4 or IPv6
	serv_addr.ai_socktype = SOCK_STREAM;
	serv_addr.ai_flags = AI_PASSIVE;     // fill in my IP for me

	getaddrinfo(serv_ip, serv_port, &serv_addr, &res);

	/* Establish the connection to the echo server */
	if (connect(sock, res->ai_addr, res->ai_addrlen) < 0){
		die_error("connect() failed");
	}

	while (c != 'q') {
		c = getchar();
		while((getchar())!='\n' && c !=EOF);
		param[0] = c;
		switch (c) {
		case 'q':
			break;
		case 'b':
//			send_response(sock, &c);
			book_flight(sock, param);
			break;
		case 'm':
		case 'v':
			send_response(sock, param);
			while ((rc=receive_message(sock, rcv_buffer)) > 0) {
				DBG("rc = %d\n", rc);
				printf("%s", rcv_buffer);
				memset(rcv_buffer, 0, MAX_BUF_SIZE);
			}

			break;
		case 'h':
		default:
			usage();
			break;
		}
		DBG("while end\n");
	}
	freeaddrinfo(res);
	close(sock);
	exit(0);
}
