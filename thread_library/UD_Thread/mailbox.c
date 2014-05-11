#include "ud_thread.h"
#include "t_lib.h"
#include <string.h>
#include <assert.h>

int
mbox_create(mbox **mb)
{
    *mb = (mbox*)calloc(1, sizeof(mbox));
    mbox *tmpmb = *mb;
    list_init(&tmpmb->msg);
    sem_init(&tmpmb->mbox_sem, 1);
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
        to_free = list_entry(e, struct messageNode, elem);
        list_remove(e);
        free(to_free->message);
        free(to_free);
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
    list_remove(e);
    free(currnode->message);
    free(currnode);
}
