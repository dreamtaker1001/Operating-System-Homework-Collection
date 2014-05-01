#include "t_lib.h"
#include "ud_thread.h"
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>

#define TIME 1 

struct list q_running;
struct list q_ready_H;
struct list q_ready_L;

void 
sighand(int sig)
{
    printf("SIGALRM caught\n");
    t_yield();
}

void 
t_yield(void)
{
    ualarm(0, 0);
    ucontext_t *curr;
    ucontext_t *next;
    tcb *running, *tcbnext;
    assert(!is_list_empty(&q_running));

    if (is_list_empty(&q_ready_H) && is_list_empty(&q_ready_L)) {
        printf("Q_ready_H and L are both empty!\n");
        ualarm(TIME, 0);
        return;
    }
    sighold(SIGALRM);

    /* Move the running thread to tail of ready queue */
    running = list_entry(list_begin(&q_running), tcb, elem);
    if (is_list_empty(&q_ready_H) && running->priority == 0) {
        printf("Q_ready_H is empty and trying to insert H thread, not doing this!\n");
        ualarm(TIME, 0);
        return;
    }
    tcb *tmpthread = (tcb*)calloc(1, sizeof(tcb));
    assert(tmpthread);
    getcontext(&tmpthread->context);
    tmpthread->context = running->context;
    assert(running->context.uc_stack.ss_sp); 
    tmpthread->thread_id = running->thread_id;
    tmpthread->priority = running->priority;
    tmpthread->thread_magic = running->thread_magic;

    int queueflag = -1;

    assert (tmpthread->priority == 0 || tmpthread->priority == 1);
    if (tmpthread->priority == 0) {
        list_insert_tail(&q_ready_H, &tmpthread->elem);
        curr = &(list_entry(list_end(&q_ready_H), tcb, elem)->context);
    }
    else if (tmpthread->priority == 1) {
        list_insert_tail(&q_ready_L, &tmpthread->elem);
        curr = &(list_entry(list_end(&q_ready_L), tcb, elem)->context);
    }
    struct list_elem *e = list_begin(&q_running);
    assert(is_thread(e));
    tcb *to_free = list_entry(e, tcb, elem);
    list_remove(e);
    free(to_free);

    /* Move the 1st thread of the ready queue into running */
    if (!is_list_empty(&q_ready_H)) {
        tcbnext = list_entry(list_begin(&q_ready_H), tcb, elem);
        queueflag = 0;
    }
    else if (!is_list_empty(&q_ready_L)) {
        tcbnext = list_entry(list_begin(&q_ready_L), tcb, elem);
        queueflag = 1;
    }
    tcb *tmpthread1 = (tcb*)calloc(1, sizeof(tcb));
    assert(tmpthread1);
    tmpthread1->context = tcbnext->context;
    assert(tmpthread1->context.uc_stack.ss_sp);  
    tmpthread1->thread_id = tcbnext->thread_id;
    tmpthread1->priority = tcbnext->priority;
    tmpthread1->thread_magic = tcbnext->thread_magic;
    list_insert_head(&q_running, &tmpthread1->elem);
    next = &(list_entry(list_begin(&q_running), tcb, elem)->context);

    assert(queueflag == 0 || queueflag == 1);
    struct list_elem *f = NULL;
    if (queueflag == 0) 
        f = list_begin(&q_ready_H);
    else if (queueflag == 1) 
        f = list_begin(&q_ready_L);
    assert(f && is_thread(f));
    tcb *to_free2 = list_entry(f, tcb, elem);
    list_remove(f);
    free(to_free2);
    sigrelse(SIGALRM);
    printf("Ready to swapcontext...\n");
    
    assert(curr);
    assert(next);
    ualarm(TIME, 0);
    if (swapcontext(curr, next) == -1) {
        printf("Swapcontext error: %s\n", strerror(errno));   
    }
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
    list_init(&q_running);
    list_init(&q_ready_H);
    list_init(&q_ready_L);

    size_t sz = 0x10000;
    tcb *tmpthread = (tcb*)calloc(1, sizeof(tcb));
    /* initializing the main thread */
    /* Previous SERIOUS bug is from here!!! */
    tmpthread->context.uc_stack.ss_sp = calloc(1, sz);
    tmpthread->context.uc_stack.ss_size = sz;
    tmpthread->context.uc_stack.ss_flags = 0;
    tmpthread->context.uc_link = NULL;
    getcontext(&tmpthread->context);
    assert(tmpthread->context.uc_stack.ss_sp);

    tmpthread->priority = 1;
    tmpthread->thread_id = -1;
    tmpthread->thread_magic = MAGIC;
    list_insert_head(&q_running, &tmpthread->elem);
    assert(signal(SIGALRM, sighand) != SIG_ERR);
    printf("Init finished!\n");
    ualarm(TIME, 0);
}

void
t_terminate()
{
    printf("Running terminate\n");
    sighold(SIGALRM);
    ucontext_t *next;
    int queueflag = 0;
    if (!is_list_empty(&q_ready_H)) {
        queueflag = 0;
        next = &(list_entry(list_begin(&q_ready_H), tcb, elem)->context);
    }
    else if (!is_list_empty(&q_ready_L)) {
        queueflag = 1;
        next = &(list_entry(list_begin(&q_ready_L), tcb, elem)->context);
    }
    else
        exit(0);
    struct list_elem *e = list_begin(&q_running);
    assert(is_thread(e));
    list_remove(e);
    free(list_entry(e, tcb, elem));
    tcb *tmpthread = (tcb*)calloc(1, sizeof(tcb));
    /* problem: why must I build a new struct tmpthread, and 
     * insert(tmpthread->elem)? why can't I just do
     * insert(list_begin(&q_ready))  ????
     */
    if (queueflag == 0)
        *tmpthread = *list_entry(list_begin(&q_ready_H), tcb, elem);
    else
        *tmpthread = *list_entry(list_begin(&q_ready_L), tcb, elem);
        
    list_insert_head(&q_running, &tmpthread->elem);
    if (queueflag == 0)
        e = list_begin (&q_ready_H);
    else
        e = list_begin (&q_ready_L);
    list_remove(e);
    ualarm(TIME, 0);
    sigrelse(SIGALRM);

    setcontext(next);
}

void
t_shutdown()
{
    printf("Beginning shutdown\n");
    ualarm(0, 0);
    struct list_elem *e;
    tcb *tmp;
    while(!is_list_empty(&q_ready_H)) {
        tmp = list_entry(list_begin(&q_ready_H), tcb, elem);
        e = list_begin(&q_ready_H);
        assert(is_thread(e));
        list_remove(e);
        free(tmp);
    }
    while(!is_list_empty(&q_ready_L)) {
        tmp = list_entry(list_begin(&q_ready_L), tcb, elem);
        e = list_begin(&q_ready_L);
        assert(is_thread(e));
        list_remove(e);
        free(tmp);
    }
    while(!is_list_empty(&q_running)) {
        tmp = list_entry(list_begin(&q_running), tcb, elem);
        e = list_begin(&q_running);
        assert(is_thread(e));
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
    uc->uc_link = NULL;
    makecontext(uc, fct, 1, id);

    tmpthread->thread_id = id;
    tmpthread->priority = pri;
    tmpthread->thread_magic = MAGIC;
    assert(is_thread(&tmpthread->elem));
    if (pri == 0) {
        list_insert_tail(&q_ready_H, &tmpthread->elem);
        assert(is_thread(list_end(&q_ready_H)));
    }
    else if (pri == 1) {
        list_insert_tail(&q_ready_L, &tmpthread->elem);
        assert(is_thread(list_end(&q_ready_L)));
    }
    printf("Created a new thread ID = %d\n", id);
    sigrelse(SIGALRM);

}
