CC=gcc
FLAGS=-g

all: shell.o cycle.o cmd.o 
	$(CC) $(FLAGS) shell.o cycle.o cmd.o -o yuqish

shell: shell.h shell.c
	$(CC) $(FLAGS) -c shell.c -o shell.o

cycle: shell.h cmd.h cycle.c
	$(CC) $(FLAGS) -c cycle.c -o cycle.o

cmd: shell.h cmd.h cmd.c
	$(CC) $(FLAGS) -c cmd.c -o cmd.o

clean:
	rm -f yuqish *.o
