FLAGS=-Wall -pedantic -L.
EXEC=trabalho3
CC=gcc

all: ring.o chat.o main.c
	$(CC) $(FLAGS) -o $(EXEC) main.c ring.o chat.o -lpthread

ring.o: ring.c ring.h
	$(CC) $(FLAGS) -c ring.c

chat.o: chat.c chat.h ring.h
	$(CC) $(FLAGS) -c chat.c -lpthread

clean: 
	rm -rf *.o $(EXEC)
