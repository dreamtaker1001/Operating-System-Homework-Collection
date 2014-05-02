#include "t_lib.h"
#include "ud_thread.h"
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

#define TIME 50 

struct list alllist;
struct list q_running;
struct list q_ready_H;
struct list q_ready_L;

void 
sighand(int sig)
{
//    printf("SIGALRM caught\n");
    t_yield();
}

void 
t_yield(void)
{
    ualarm(0, 0);
    ucontext_t *curr = NULL, *next = NULL;
    tcb *running = NULL, *tcbnext = NULL;
    struct list_elem *e = NULL;
    assert(!is_list_empty(&q_running));

    sighold(SIGALRM);
    running = list_entry(list_begin(&q_running), thread_p, elem)->p;
    if (is_list_empty(&q_ready_H) && is_list_empty(&q_ready_L)) {
        printf("Q_ready_H and L are both empty!\n");
        ualarm(TIME, 0);
        return;
    }
    if (is_list_empty(&q_ready_H) && running->priority == 0) {
        printf("Q_ready_H is empty and trying to insert H thread, not doing this!\n");
        ualarm(TIME, 0);
        return;
    }
    /*
    for (e = list_begin(&alllist); e != list_tail(&alllist); 
            e = list_next(e)) {
        assert(is_thread(e));
    }
    */
    /* Move the running thread to tail of ready queue */
    int queueflag = -1;
    thread_p *tmp_p = (thread_p *)calloc(1, sizeof(thread_p));
    tmp_p->p = running;

    if (running->priority == 0) 
        list_insert_tail(&q_ready_H, &tmp_p->elem);
    else if (running->priority == 1) 
        list_insert_tail(&q_ready_L, &tmp_p->elem);
    curr = &running->context;
    e = list_begin(&q_running);
    tmp_p = list_entry(e, thread_p, elem);
    list_remove(e);
    free(tmp_p);

    /* Move the 1st thread of the ready queue into running */
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
    printf("Ready to swapcontext...\n");
    sigrelse(SIGALRM);
    printf("After sigrelse...\n");
    
    ualarm(TIME, 0);
    printf("after ualarm is set...\n");
    assert(curr);
    assert(next);
    if (swapcontext(curr, next) == -1) {
        printf("Swapcontext error: %s\n", strerror(errno));   
    }
    printf("after swapcontext\n");
}

int
is_thread(struct list_elem *e)
{
    assert(e);
    tcb *to_check = list_entry(e, tcb, elem);
    return (to_check->thread_magic == MAGIC &&
            (to_check->priority == 0 || to_check->priority == 1));
}

void 
t_init()
{
    list_init(&alllist);
    list_init(&q_running);
    list_init(&q_ready_H);
    list_init(&q_ready_L);

    assert(signal(SIGALRM, sighand) != SIG_ERR);
    size_t sz = 0x10000;
    tcb *tmpthread = (tcb*)calloc(1, sizeof(tcb));
    /* initializing the main thread */
    /* Previous SERIOUS bug is from here!!! */
    tmpthread->context.uc_stack.ss_sp = calloc(1, sz);
    tmpthread->context.uc_stack.ss_size = sz;
    tmpthread->context.uc_stack.ss_flags = 0;
    tmpthread->context.uc_link = NULL;
    getcontext(&tmpthread->context);

    tmpthread->priority = 1;
    tmpthread->thread_id = -1;
    tmpthread->thread_magic = MAGIC;
    list_insert_head(&alllist, &tmpthread->elem);
    thread_p *newthread = (thread_p *)calloc(1, sizeof(thread_p));
    newthread->p = tmpthread;
    list_insert_head(&q_running, &newthread->elem);
    ualarm(TIME, 0);
}

void
t_terminate()
{
    sighold(SIGALRM);
    ucontext_t *next;
    tcb *tmpthread;
    int queueflag = 0;
    if (!is_list_empty(&q_ready_H)) {
        queueflag = 0;
        tmpthread = list_entry(list_begin(&q_ready_H), thread_p, elem)->p;
    }
    else if (!is_list_empty(&q_ready_L)) {
        queueflag = 1;
        tmpthread = list_entry(list_begin(&q_ready_L), thread_p, elem)->p;
    }
    else
        exit(0);
    next = &tmpthread->context;
    struct list_elem *e = list_begin(&q_running);
    thread_p *tmp_p = list_entry(e, thread_p, elem);
    tcb *tmp = tmp_p->p;
    list_remove(&tmp->elem);
    free(tmp);
    list_remove(e);
    free(tmp_p);
    /* problem: why must I build a new struct tmpthread, and 
     * insert(tmpthread->elem)? why can't I just do
     * insert(list_begin(&q_ready))  ????
     */
    tmp_p = (thread_p *)calloc(1, sizeof(thread_p));
    if (queueflag == 0) 
        e = list_begin(&q_ready_H);
    else if (queueflag == 1)
        e = list_begin(&q_ready_L);
    tmp_p->p = list_entry(e, thread_p, elem)->p;
    list_insert_head(&q_running, &tmp_p->elem);

    list_remove(e);
    free(list_entry(e, thread_p, elem));
    ualarm(TIME, 0);
    sigrelse(SIGALRM);

    setcontext(next);
}

void
t_shutdown()
{
    ualarm(0, 0);
    struct list_elem *e;
    tcb *tmp;
    thread_p *tmp_p;
    while(!is_list_empty(&q_ready_H)) {
        tmp_p = list_entry(list_begin(&q_ready_H), thread_p, elem);
        e = list_begin(&q_ready_H);
        list_remove(e);
        free(tmp_p);
    }
    while(!is_list_empty(&q_ready_L)) {
        tmp_p = list_entry(list_begin(&q_ready_L), thread_p, elem);
        e = list_begin(&q_ready_L);
        list_remove(e);
        free(tmp_p);
    }
    while(!is_list_empty(&q_running)) {
        tmp_p = list_entry(list_begin(&q_running), thread_p, elem);
        e = list_begin(&q_running);
        list_remove(e);
        free(tmp_p);
    }
    while(!is_list_empty(&alllist)) {
        tmp = list_entry(list_begin(&alllist), tcb, elem);
        e = list_begin(&alllist);
        list_remove(e);
        free(tmp);
    }
}

void 
t_create(void (*fct)(int), int id, int pri)
{
    size_t sz = 0x10000;

    sighold(SIGALRM);
    tcb *tmpthread = (tcb*)calloc(1, sizeof(tcb));
    ucontext_t *uc = &tmpthread->context;
    getcontext(uc);
    uc->uc_stack.ss_sp = calloc(1, sz);  
    uc->uc_stack.ss_size = sz;
    uc->uc_stack.ss_flags = 0;
    uc->uc_link = &list_entry(list_begin(&q_running), thread_p, elem)->p->context;
    makecontext(uc, fct, 1, id);

    tmpthread->thread_id = id;
    tmpthread->priority = pri;
    tmpthread->thread_magic = MAGIC;
    list_insert_head(&alllist, &tmpthread->elem);

    thread_p *newthread = (thread_p*)calloc(1, sizeof(thread_p));
    newthread->p = tmpthread;
    if (pri == 0) {
        list_insert_tail(&q_ready_H, &newthread->elem);
    }
    else if (pri == 1) {
        list_insert_tail(&q_ready_L, &newthread->elem);
    }
    printf("Created a new thread ID = %d\n", id);
    sigrelse(SIGALRM);
}
