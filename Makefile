all: ring.o chat.o main.c
	gcc main.c ring.o chat.o -L. -lpthread

ring.o: ring.c ring.h
	gcc -c ring.c -L.

chat.o: chat.c chat.h ring.h
	gcc -c chat.c -L. -lpthread

clean: 
	rm -rf *.o a.out
