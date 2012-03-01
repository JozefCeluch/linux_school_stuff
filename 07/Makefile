CC=gcc
CFLAGS=-Wall -pedantic -std=c99
round_buf.o class : round_buf.c soubeh.c
	gcc -Wall -fPIC -c round_buf.c
	
	gcc -shared -lpthread -Wl,-soname,librbuf.so.0 -o librbuf.so.0.0.1 round_buf.o
	
	gcc -Wall -pthread -o class soubeh.c
