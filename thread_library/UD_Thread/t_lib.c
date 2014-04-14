#include "t_lib.h"
#include <assert.h>

struct list q_running;
struct list q_ready;

void t_yield()
{
    printf("Yielding the current thread...\n");
    ucontext_t *curr, *next;
    curr = (ucontext_t*)calloc(1, sizeof(ucontext_t));
    next = (ucontext_t*)calloc(1, sizeof(ucontext_t));
    getcontext(curr);

    /* Move the running thread to tail of ready queue */
    struct thread_t *tmpthread = (struct thread_t*)calloc(1, sizeof(struct thread_t));
    tmpthread->context = curr;
    list_insert_tail(&q_ready, &tmpthread->elem);
    list_remove(list_begin(&q_running));
    /* Move the 1st thread of the ready queue into running */
    next = list_entry(list_begin(&q_ready), struct thread_t, elem)->context;
    struct thread_t *tmpthread1 = (struct thread_t*)calloc(1, sizeof(struct thread_t));
    tmpthread1->context = next;
    list_insert_head(&q_running, &tmpthread1->elem);
    list_remove(list_begin(&q_ready));

    setcontext(next);
}

void t_init()
{
    list_init(&q_running);
    list_init(&q_ready);
    assert(is_list_empty(&q_running));
    assert(is_list_empty(&q_ready));

    struct thread_t *tmpthread;
    tmpthread = (struct thread_t*)calloc(1, sizeof(struct thread_t));
    tmpthread->context = (ucontext_t*)calloc(1, sizeof(ucontext_t));
    getcontext(tmpthread->context);
    list_insert_head(&q_running, &tmpthread->elem);
}

void
t_terminate()
{
    
}

int t_create(void (*fct)(int), int id, int pri)
{
    size_t sz = 0x10000;

    ucontext_t *uc;
    uc = (ucontext_t *)calloc(1, sizeof(ucontext_t));

    getcontext(uc);
    uc->uc_stack.ss_sp = calloc(1, sz);  
    uc->uc_stack.ss_size = sz;
    uc->uc_stack.ss_flags = 0;
    uc->uc_link = 0;
    makecontext(uc, fct, 1, id);

    struct thread_t *tmpthread = (struct thread_t*)calloc(1, sizeof(struct thread_t));
    struct list_elem *e;
    tmpthread->context = uc;
    e = &tmpthread->elem;
    list_insert_tail(&q_ready, e);
}
