FLAGS=-Wall -g -pedantic
EXEC=trabalho3
CC=gcc
WAITFLAG=-DWAIT=1

wait: ring-wait.o chat.o main.c
	$(CC) $(FLAGS) -o $(EXEC) main.c ring-wait.o chat.o -L. -lpthread

ring-wait.o: ring.c ring.h
	$(CC) $(FLAGS) $(WAITFLAG) -o ring-wait.o -c ring.c -L.

nowait: ring.o chat.o main.c
	$(CC) $(FLAGS) -o $(EXEC) main.c ring.o chat.o -L. -lpthread

ring.o: ring.c ring.h
	$(CC) $(FLAGS) -c ring.c -L.

chat.o: chat.c chat.h ring.h
	$(CC) $(FLAGS) -c chat.c -L. -lpthread

clean: 
	rm -rf *.o $(EXEC)
