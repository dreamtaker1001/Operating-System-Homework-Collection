# Makefile for UD CISC user-level thread library

CC = gcc
CFLAGS = -g -lmcheck

LIBOBJS = t_lib.o list.o

TSTOBJS = test01.o test01x.o 

# specify the executable 

EXECS = test01 test01x

# specify the source files

LIBSRCS = t_lib.c list.c

TSTSRCS = test01.c test01x.c
TSTEXES = test01 test01x
TSTOUTS = 1.txt 1x.txt

# ar creates the static thread library

t_lib.a: ${LIBOBJS} Makefile
	ar rcs t_lib.a ${LIBOBJS}

# here, we specify how each file should be compiled, what
# files they depend on, etc.

test: ${TSTEXES} ${TSTOUTS}
	./test01 | diff 1.txt - > /dev/null && echo "test01 passed~" || echo "test01 FAILED!"
	./test01x | diff 1x.txt - > /dev/null && echo "test01x passed~" || echo "test01x FAILED!"

t_lib.o: t_lib.c t_lib.h Makefile
	${CC} ${CFLAGS} -c t_lib.c

list.o:	list.h list.c Makefile
	${CC} ${CFLAGS} -c list.c

test01.o: test01.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test01.c

test01x.o: test01x.c ud_thread.h Makefile
	${CC} ${CFLAGS} -c test01x.c

test01: test01.o t_lib.a Makefile
	${CC} ${CFLAGS} test01.o t_lib.a -o test01

test01x: test01x.o t_lib.a Makefile
	${CC} ${CFLAGS} test01x.o t_lib.a -o test01x

clean:
	rm -f t_lib.a ${EXECS} ${LIBOBJS} ${TSTOBJS} 
