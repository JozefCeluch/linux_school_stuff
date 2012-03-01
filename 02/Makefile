# check for c compiler
CC=gcc
CFLAGS=-Wall -pedantic -std=c99
prog hello1.o hello2.o : hello1.c hello2.c prog.c
	gcc -Wall -fPIC -c hello1.c
	gcc -Wall -fPIC -c hello2.c
	
	gcc -shared -Wl,-soname,libhello2.so.0 -o libhello2.so.0.0.1 hello2.o
	gcc -shared -Wl,-soname,libhello1.so.0 -o libhello1.so.0.0.1 hello1.o

#prog : prog.c
	gcc -ldl -o prog prog.c
