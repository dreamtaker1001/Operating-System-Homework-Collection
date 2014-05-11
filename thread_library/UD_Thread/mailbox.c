#include "ud_thread.h"
#include "t_lib.h"
#include <string.h>
#include <assert.h>

int global_ready = 0;
static mbox *global_mbox;
extern struct list q_running;

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

void
send(int tid, char *msg, int len)
{
    if (global_ready == 0) {
        mbox_create(&global_mbox);
        assert(global_mbox->mbox_sem);
        global_ready = 1;
    }
    
    struct messageNode *newnode = (struct messageNode*) 
        calloc(1, sizeof(struct messageNode));
    newnode->message = (char*)calloc(1, len+1);
    strcpy(newnode->message, msg);
    newnode->len = len;
    newnode->sender = curr_tid();
    newnode->receiver = tid;

    assert(global_mbox);
    assert(global_mbox->mbox_sem);
    sem_wait(global_mbox->mbox_sem);
    list_insert_tail(&global_mbox->msg, &newnode->elem);
    sem_signal(global_mbox->mbox_sem);
}

void
receive(int *tid, char *msg, int *len)
{
    int curr = curr_tid();
    int flag = 0;
    struct list *tmplist = &(global_mbox->msg);
    struct list_elem *e = list_begin(tmplist);
    struct messageNode *tmpnode = NULL;
    
    while(is_interior(e)) {
        tmpnode = list_entry(e, struct messageNode, elem);
        if (*tid == 0 || *tid == tmpnode->sender) {
            flag = 1;
            break;
        }
        e = list_next(e);
    }
    if (flag == 0) {
        *tid = 0;
        *len = 0;
        return;
    }
    else {
        strcpy(msg, tmpnode->message);
        *tid = tmpnode->sender;
        *len = tmpnode->len;
    }
    free_node(e);
}

void
block_send(int tid, char *msg, int length)
{

}

void
block_receive(int *tid, char *msg, int *length)
{

}
