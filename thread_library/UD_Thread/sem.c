#include "ud_thread.h"
#include "t_lib.h"

int 
sem_init(sem_t **sp, unsigned int count)
{
    *sp = (sem*)malloc(sizeof(sem));
    list_init(&(((sem*)*sp)->waiters));
    ((sem*)*sp)->value = count;
}

void 
sem_wait(sem_t *sp)
{

}

void 
sem_signal(sem_t *sp)
{

}

void 
sem_destroy(sem_t **sp)
{
    struct list_elem *e = NULL;
    sem *sema = *sp;
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
