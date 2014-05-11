/* 
 * thread library function prototypes
 */
#ifndef H_UD_THREAD
#define H_UD_THREAD
#include "list.h"
typedef void sem_t;  // for semaphore

struct messageNode {
    char *message;     // copy of the message 
    int  len;          // length of the message 
    int  sender;       // TID of sender thread 
    int  receiver;     // TID of receiver thread 
    struct list_elem elem;
};

typedef struct {
    struct list msg;
    sem_t *mbox_sem;
} mbox;   // for mailbox

void 
t_create(void(*function)(int), int thread_id, int priority);

void 
t_yield(void);

void 
t_init(void);

void
t_terminate(void);

void
t_shutdown(void);

int sem_init(sem_t **sp, int count);
void sem_wait(sem_t *sp);
void sem_signal(sem_t *sp);
void sem_destroy(sem_t **sp);

int mbox_create(mbox **mb); 
void mbox_destroy(mbox **mb);
void mbox_deposit(mbox *mb, char *msg, int len);
void mbox_withdraw(mbox *mb, char *msg, int *len);

#endif
