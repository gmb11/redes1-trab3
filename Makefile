FLAGS=-Wall -pedantic
LIBS=-L. -lpthread
EXEC=trabalho3
CC=gcc

all: ring.o chat.o main.c
	$(CC) $(FLAGS) -o $(EXEC) main.c ring.o chat.o $(LIBS)

ring.o: ring.c ring.h
	$(CC) $(FLAGS) -c ring.c $(LIBS)

chat.o: chat.c chat.h ring.h
	$(CC) $(FLAGS) -c chat.c $(LIBS)

clean: 
	rm -rf *.o $(EXEC)
