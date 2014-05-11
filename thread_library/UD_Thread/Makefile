# Makefile for UD CISC user-level thread library

CC = gcc
#CFLAGS = -g -O0
CFLAGS = -g -lmcheck -O0 

LIBOBJS = t_lib.o list.o sem.o mailbox.o

TSTOBJS = test06.o 

# specify the executable 

EXECS = test06

# specify the source files

LIBSRCS = t_lib.c list.c sem.c mailbox.c

TSTSRCS = test06.c
TSTEXES = test06 
TSTOUTS = 6.txt 

# ar creates the static thread library

t_lib.a: ${LIBOBJS} Makefile
	ar rcs t_lib.a ${LIBOBJS}

# here, we specify how each file should be compiled, what
# files they depend on, etc.

test: ${TSTEXES} ${TSTOUTS}
	./test06 | diff 6.txt - > /dev/null && echo "test06 passed~" || echo "test06 FAILED!"

t_lib.o: t_lib.c t_lib.h Makefile
	${CC} ${CFLAGS} -c t_lib.c 

sem.o: sem.c t_lib.h ud_thread.h Makefile
	${CC} ${CFLAGS} -c sem.c

list.o:	list.h list.c Makefile
	${CC} ${CFLAGS} -c list.c

mailbox.o: mailbox.c t_lib.h ud_thread.h Makefile
	${CC} ${CFLAGS} -c mailbox.c

test06.o: test06.c ud_thread.h list.h t_lib.h
	${CC} ${CFLAGS} -c test06.c

test06: test06.o t_lib.a
	${CC} ${CFLAGS} test06.o t_lib.a -o test06

clean:
	rm -f t_lib.a ${EXECS} ${LIBOBJS} ${TSTOBJS} 
