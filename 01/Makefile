# check for c compiler
CC=gcc
CFLAGS=-Wall -pedantic -std=c99
file.o : file.c
	$(CC) -Wall -fPIC -c file.c
	$(CC) -shared -Wl,-soname,libmytime.so.0 -o libmytime.so.0.0.1 file.o


