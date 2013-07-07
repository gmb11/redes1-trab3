all: ring.o main.c
	gcc main.c ring.o -L.

ring.o: ring.c ring.h
	gcc -c ring.c -L.
