CC=gcc
FLAGS=-g

all: init.o
	$(CC) $(FLAGS) init.o -o queue

init: queue.h init.c
	$(CC) $(FLAGS) -c init.c -o init.o

clean:
	rm -f *.o queue
