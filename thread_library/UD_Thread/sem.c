#include "ud_thread.h"
#include "t_lib.h"
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

extern struct list alllist;
extern struct list q_running;
extern struct list q_ready_H;
extern struct list q_ready_L;

int 
sem_init(sem_t **sp, int count)
{
    *sp = (sem*)calloc(1, sizeof(sem));
    list_init(&(((sem*)*sp)->waiters));
    ((sem*)*sp)->value = count;
    ((sem*)*sp)->flag = 0;
}

int
sem_try_lock(sem_t *sp)
{
    if (((sem*)sp)->flag == 0) {
        ((sem*)sp)->flag = 1;
        return 0;
    }
    else return 1;
}

void
sem_unlock(sem_t *sp)
{
    ((sem*)sp)->flag = 0;
}

void
sem_yield(sem_t *sp)
{
    sem *sema = (sem*)sp;
    ucontext_t *curr = NULL, *next = NULL;
    tcb *running = NULL, *tcbnext = NULL;
    struct list_elem *e = NULL;
    int queueflag = -1;

    running = list_entry(list_begin(&q_running), thread_p, elem)->p;
    if (is_list_empty(&q_ready_H) && is_list_empty(&q_ready_L)) {
        printf("Q_ready_H and L are both empty!\n");
        return;
    }

    thread_p *tmp_p = (thread_p *)calloc(1, sizeof(thread_p));
    tmp_p->p = running;
    list_insert_tail(&sema->waiters, &tmp_p->elem);
    curr = &running->context;
    e = list_begin(&q_running);
    tmp_p = list_entry(e, thread_p, elem);
    list_remove(e);
    free(tmp_p);

    if (!is_list_empty(&q_ready_H)) {
        tcbnext = list_entry(list_begin(&q_ready_H), thread_p, elem)->p;
        queueflag = 0;
    }
    else if (!is_list_empty(&q_ready_L)) {
        tcbnext = list_entry(list_begin(&q_ready_L), thread_p, elem)->p;
        queueflag = 1;
    }
    tmp_p = (thread_p *)calloc(1, sizeof(thread_p));
    tmp_p->p = tcbnext;
    list_insert_head(&q_running, &tmp_p->elem);
    next = &tcbnext->context;

    if (queueflag == 0) 
        e = list_begin(&q_ready_H);
    else if (queueflag == 1) 
        e = list_begin(&q_ready_L);
    tmp_p = list_entry(e, thread_p, elem);
    list_remove(e);
    free(tmp_p);
    assert(curr);
    assert(next);
    if (swapcontext(curr, next) == -1) {
        printf("Swapcontext error: %s\n", strerror(errno));   
    }
}

void 
sem_wait(sem_t *sp)
{
    /*Possible bug point*/
    while(sem_try_lock(sp) != 0);
    ((sem*)sp)->value--;
    if ( ((sem*)sp)->value < 0 ) {
        sem_unlock(sp);
        sem_yield(sp);
    }
    sem_unlock(sp);
}

void
sem_unblock(sem_t *sp)
{
    sem *sema = (sem*)sp;
    tcb *tcbnext = NULL;
    struct list_elem *e = NULL;

    thread_p *tmp_p = (thread_p *)calloc(1, sizeof(thread_p));
    tmp_p->p = list_entry(list_begin(&sema->waiters), thread_p, elem)->p;
    if (tmp_p->p->priority == 0) 
        list_insert_tail(&q_ready_H, &tmp_p->elem);
    else if (tmp_p->p->priority == 1)
        list_insert_tail(&q_ready_L, &tmp_p->elem);
    e = list_begin(&sema->waiters);
    tmp_p = list_entry(e, thread_p, elem);
    list_remove(e);
    free(tmp_p);
}

void 
sem_signal(sem_t *sp)
{
    while(sem_try_lock(sp) != 0);
    int yield = 0;
    ((sem*)sp)->value++;
    if ( ((sem*)sp)->value <= 0 ) {
        assert(!is_list_empty(&((sem*)sp)->waiters));
        sem_unblock(sp);
        yield = 1;
    }
    sem_unlock(sp);
    if (yield == 1)
        t_yield();
}

void 
sem_destroy(sem_t **sp)
{
    struct list_elem *e = NULL;
    sem *sema = (sem*)*sp;
    tcb *tcbtmp = NULL;
    struct list *list = &sema->waiters;
    while (!is_list_empty(list)) {
        e = list_begin(list);
        tcbtmp = list_entry(e, tcb, elem);
        list_remove(e);
        free(tcbtmp->context.uc_stack.ss_sp);
        free(tcbtmp);
    }
    sp = NULL;
}
