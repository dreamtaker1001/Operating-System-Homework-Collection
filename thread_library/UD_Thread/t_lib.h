/*
 * types used by thread library
 */
#ifndef H_T_LIB
#define H_T_LIB
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include "list.h"

#define MAGIC 0x123456

struct tcb {
    int thread_id;
    int priority;
    ucontext_t context;
    int thread_magic;
    struct list_elem elem;
};
typedef struct tcb tcb;

struct thr {
    tcb *p;
    struct list_elem elem;
};

typedef struct thr thread_p;

struct semaphore {
    unsigned int value;
    struct list waiters;     
};

typedef struct semaphore sem;

void
sighand(int);

int
is_thread(struct list_elem*);

#endif
