CC=gcc
CFLAGS= -g -O0
#allflag=-g -lpthread -lkstat -DHAVE_KSTAT
allflag= -g -O0 -lpthread -lmcheck -DHAVE_KSTAT 

all: shell.o cycle.o dir.o self.o env.o which.o signal.o exec.o bg.o warnload.o watch.o mail.o rd.o ipc.o
	$(CC) $(allflag) shell.o cycle.o dir.o self.o env.o which.o signal.o exec.o bg.o warnload.o watch.o mail.o rd.o ipc.o -o yuqish

shell: shell.h shell.c
	$(CC) $(CFLAGS) shell.c -o shell.o

cycle: shell.h cmd.h cycle.c
	$(CC) $(CFLAGS) cycle.c -o cycle.o

dir: shell.h cmd.h dir.c
	$(CC) $(CFLAGS) dir.c -o dir.o

self: shell.h cmd.h self.c
	$(CC) $(CFLAGS) self.c -o self.o

env: shell.h cmd.h env.c
	$(CC) $(CFLAGS) env.c -o env.o

which: shell.h cmd.h which.c
	$(CC) $(CFLAGS) which.c -o which.o

signal: shell.h cmd.h sighand.h signal.c
	$(CC) $(CFLAGS) signal.c -o signal.o

exec: shell.h cmd.h exec.c
	$(CC) $(CFLAGS) exec.c -o exec.o

bg: util.h shell.h cmd.h bg.c
	$(CC) $(CFLAGS) bg.c -o bg.o

warnload: util.h shell.h cmd.h warnload.c
	$(CC) $(CFLAGS) warnload.c -o warnload.o

watch: util.h shell.h cmd.h watch.c
	$(CC) $(CFLAGS) watch.c -o watch.o
	
mail: util.h shell.h cmd.h mail.c
	$(CC) $(CFLAGS) mail.c -o mail.o

rd: util.h shell.h cmd.h rd.c
	$(CC) $(CFLAGS) rd.c -o rd.o

ipc: util.h shell.h cmd.h ipc.c
	$(CC) $(CFLAGS) ipc.c -o ipc.o

clean:
	rm -f yuqish *.o
