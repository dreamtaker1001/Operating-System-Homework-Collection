#include "ud_thread.h"
#include "t_lib.h"
#include <string.h>
#include <assert.h>

extern struct list q_running;
extern struct list alllist;

int
mbox_create(mbox **mb)
{
    *mb = (mbox*)calloc(1, sizeof(mbox));
    mbox *tmpmb = *mb;
    list_init(&tmpmb->msg);
    sem_init(&tmpmb->mbox_sem, 1);
    assert(tmpmb->mbox_sem);
    return 0;
}

void
mbox_destroy(mbox **mb)
{
    mbox *tmpmb = *mb;
    struct messageNode *to_free = NULL;
    struct list_elem *e = NULL;
    while(!is_list_empty(&tmpmb->msg)) {
        e = list_begin(&tmpmb->msg);
        free_node(e);
    }
    sem_destroy(&tmpmb->mbox_sem);
    free(tmpmb);
}

void
mbox_deposit(mbox *mb, char *msg, int len)
{
    struct messageNode *newnode = (struct messageNode*) 
        calloc(1, sizeof(struct messageNode));
    newnode->message = (char*)calloc(1, len+1);
    strcpy(newnode->message, msg);
    newnode->len = strlen(msg);

    sem_wait(mb->mbox_sem);
    list_insert_tail(&mb->msg, &newnode->elem);
    sem_signal(mb->mbox_sem);
}

void
mbox_withdraw(mbox *mb, char *msg, int *len)
{
    struct messageNode *currnode = NULL;
    struct list_elem *e = NULL;
    e = list_begin(&mb->msg);
    assert(!is_list_empty(&mb->msg));
    currnode = list_entry(e, struct messageNode, elem);
    assert(msg);
    strcpy(msg, currnode->message);
    *len = currnode->len;
    /*Should remove the first message*/
    free_node(e);
}

void
free_node(struct list_elem *e)
{
    struct messageNode *currnode = NULL;
    currnode = list_entry(e, struct messageNode, elem);
    list_remove(e);
    free(currnode->message);
    free(currnode);
}

/*get the current tid*/
int
curr_tid(void)
{
    struct list_elem *e = list_begin(&q_running);
    thread_p *p = list_entry(e, thread_p, elem);
    return p->p->thread_id;
}

struct list_elem *
locate_tid(int tid)
{
    struct list_elem *e = list_begin(&alllist);
    int flag = 0;
    tcb *tcbtmp = NULL;
    while(is_interior(e)) {
        tcbtmp = list_entry(e, tcb, elem);
        if (tcbtmp->thread_id == tid)
            return e;
        e = list_next(e);
    }
    printf("ERROR: tid not found!\n");
    return NULL;
}

void
send(int tid, char *msg, int len)
{
    struct messageNode *newnode = (struct messageNode*) 
        calloc(1, sizeof(struct messageNode));
    newnode->message = (char*)calloc(1, len+1);
    strcpy(newnode->message, msg);
    newnode->len = len;
    newnode->sender = curr_tid();
    newnode->receiver = tid;

    struct list_elem *e = locate_tid(tid);
    tcb *tcbtmp = list_entry(e, tcb, elem);
    list_insert_tail(&tcbtmp->messagequeue, &newnode->elem);
}

void
receive(int *tid, char *msg, int *len)
{
    int curr = curr_tid();
    int flag = 0;

    struct list_elem *e = locate_tid(curr);
    struct list *tmplist = NULL;
    tmplist = &list_entry(e, tcb, elem)->messagequeue;
    e = list_begin(tmplist);
    struct messageNode *tmpnode = NULL;
    
    if (is_interior(e)) {
        tmpnode = list_entry(e, struct messageNode, elem);
        strcpy(msg, tmpnode->message);
        *tid = tmpnode->sender;
        *len = tmpnode->len;
    }
    else {
        *tid = 0;
        *len = 0;
        return;
    }
    free_node(e);
}

void
check_sem()
{
    struct list_elem *e = NULL;
    tcb *tmp = NULL;
    e = list_begin(&alllist);
    while(is_interior(e)) {
        tmp = list_entry(e, tcb, elem);
        printf("Thread %d's sem info: send-(%d), recv-(%d).\n", tmp->thread_id, ((sem*)(tmp->sem_block_sender))->value, ((sem*)(tmp->sem_block_receiver))->value);
        e = list_next(e);
    }
}

void
block_send(int tid, char *msg, int length)
{
    struct messageNode *newnode = (struct messageNode*) 
        calloc(1, sizeof(struct messageNode));
    newnode->message = (char*)calloc(1, length+1);
    strcpy(newnode->message, msg);
    newnode->len = length;
    newnode->sender = curr_tid();
    newnode->receiver = tid;

    struct list_elem *e = locate_tid(tid);
    tcb *tcbtmp = list_entry(e, tcb, elem);
    e = locate_tid(curr_tid());
    tcb *tcbself = list_entry(e, tcb, elem);
    list_insert_tail(&tcbtmp->messagequeue, &newnode->elem);

    sem_signal(tcbtmp->sem_block_receiver);
    sem_wait(tcbself->sem_block_sender);
}

void
block_receive(int *tid, char *msg, int *length)
{
    int curr = curr_tid();
    int flag = 0;

    struct list_elem *e = locate_tid(curr);
    tcb *tcbtmp = list_entry(e, tcb, elem);
    struct list *tmplist = NULL;
    tmplist = &tcbtmp->messagequeue;
    e = list_begin(tmplist);
    struct messageNode *tmpnode = NULL;

    while(1) {
        e = list_begin(tmplist);
        if (!is_interior(e)) {
            sem_wait(tcbtmp->sem_block_receiver);
        }
        else {
            assert(is_interior(e));
            tmpnode = list_entry(e, struct messageNode, elem);
            strcpy(msg, tmpnode->message);
            *tid = tmpnode->sender;
            *length = tmpnode->len;
            break;
        }
    }
    free_node(e);
    e = locate_tid(*tid);
    tcb *sender = list_entry(e, tcb, elem);
    sem_signal(sender->sem_block_sender);
}
