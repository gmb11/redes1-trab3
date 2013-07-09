FLAGS=-Wall -g -pedantic
EXEC=trabalho3
CC=gcc

all: ring.o chat.o main.c
	$(CC) $(FLAGS) -o $(EXEC) main.c ring.o chat.o -L. -lpthread

ring.o: ring.c ring.h
	$(CC) $(FLAGS) -c ring.c -L.

chat.o: chat.c chat.h ring.h
	$(CC) $(FLAGS) -c chat.c -L. -lpthread

clean: 
	rm -rf *.o $(EXEC)
