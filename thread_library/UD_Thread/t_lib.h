/*
 * types used by thread library
 */
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>
#include "list.h"

struct thread_t {
    struct list_elem elem;
    ucontext_t *context;
};
