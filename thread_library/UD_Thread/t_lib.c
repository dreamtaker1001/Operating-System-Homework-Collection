#include "t_lib.h"
#include <assert.h>

struct list q_running;
struct list q_ready;

void t_yield()
{
    ucontext_t *curr;
    ucontext_t *next;
    assert(!is_list_empty(&q_running));

    /* Move the running thread to tail of ready queue */
    tcb *tmpthread = (tcb*)calloc(1, sizeof(tcb));
    getcontext(&tmpthread->context);
    list_insert_tail(&q_ready, &tmpthread->elem);
    list_remove(list_begin(&q_running));
    curr = &list_entry(list_end(&q_ready), tcb, elem)->context;
    /* Move the 1st thread of the ready queue into running */
    next = &list_entry(list_begin(&q_ready), tcb, elem)->context;
    tcb *tmpthread1 = (tcb*)calloc(1, sizeof(tcb));
    tmpthread1->context = *next;
    list_insert_head(&q_running, &tmpthread1->elem);
    next = &list_entry(list_begin(&q_running), tcb, elem)->context;
    list_remove(list_begin(&q_ready));

    swapcontext(curr, next);
}

void t_init()
{
    list_init(&q_running);
    list_init(&q_ready);
    assert(is_list_empty(&q_running));
    assert(is_list_empty(&q_ready));

    tcb *tmpthread;
    tmpthread = (tcb*)calloc(1, sizeof(tcb));
    getcontext(&tmpthread->context);
    list_insert_head(&q_running, &tmpthread->elem);
}

void
t_terminate()
{
    ucontext_t *next;
    next = &list_entry(list_begin(&q_ready), tcb, elem)->context;
    struct list_elem *e = list_begin(&q_running);
    list_remove(e);
    free(list_entry(e, tcb, elem));
    tcb *tmpthread = (tcb*)calloc(1, sizeof(tcb));
    /* problem: why must I build a new struct tmpthread, and 
     * insert(tmpthread->elem)? why can't I just do
     * insert(list_begin(&q_ready))  ????
     */
    *tmpthread = *list_entry(list_begin(&q_ready), tcb, elem);
    list_insert_head(&q_running, &tmpthread->elem);
    list_remove(list_begin(&q_ready));
    setcontext(next);
}

void
t_shutdown()
{
    struct list_elem *e;
    tcb *tmp;
    e = list_begin(&q_ready);
    while(!is_list_empty(&q_ready)) {
        tmp = list_entry(list_begin(&q_ready), tcb, elem);
        list_remove(e);
        free(tmp);
    }
    e = list_begin(&q_running);
    while(!is_list_empty(&q_running)) {
        tmp = list_entry(list_begin(&q_running), tcb, elem);
        list_remove(e);
        free(tmp);
    }
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

    tcb *tmpthread = (tcb*)calloc(1, sizeof(tcb));
    tmpthread->context = *uc;
    tmpthread->thread_id = id;
    tmpthread->priority = pri;
    list_insert_tail(&q_ready, &tmpthread->elem);
    assert(!is_list_empty(&q_ready));
}
