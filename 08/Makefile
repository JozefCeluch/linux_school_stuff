CC=gcc
CFLAGS=-pthread -Wall
all:server.c main_server.c main_client.c prog.c
	gcc -pthread -Wall -o server main_server.c server.c
	gcc -Wall -o client main_client.c
	gcc -Wall -o class prog.c
	
class:prog.c
	gcc -Wall -o class prog.c
	
client:main_client.c
	gcc -Wall -o client main_client.c
	
server:main_server.c
	gcc -pthread -Wall -o server main_server.c server.c
