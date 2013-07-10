FLAGS=-Wall -pedantic -L. -lpthread
EXEC=trabalho3
CC=gcc

all: ring.o chat.o main.c
	$(CC) $(FLAGS) -o $(EXEC) main.c ring.o chat.o

ring.o: ring.c ring.h
	$(CC) $(FLAGS) -c ring.c

chat.o: chat.c chat.h ring.h
	$(CC) $(FLAGS) -c chat.c

clean: 
	rm -rf *.o $(EXEC)
