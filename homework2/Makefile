CC=gcc
FLAGS=-g

all: shell.o cycle.o dir.o self.o env.o which.o 
	$(CC) $(FLAGS) shell.o cycle.o dir.o self.o env.o which.o -o yuqish

shell: shell.h shell.c
	$(CC) $(FLAGS) -c shell.c -o shell.o

cycle: shell.h cmd.h cycle.c
	$(CC) $(FLAGS) -c cycle.c -o cycle.o

dir: shell.h cmd.h dir.c
	$(CC) $(FLAGS) -c dir.c -o dir.o

self: shell.h cmd.h self.c
	$(CC) $(FLAGS) -c self.c -o self.o

env: shell.h cmd.h env.c
	$(CC) $(FLAGS) -c env.c -o env.o

which: shell.h cmd.h which.c
	$(CC) $(FLAGS) -c which.c -o which.o

clean:
	rm -f yuqish *.o
