#ifndef H_QUEUE
#define H_QUEUE

struct list_elem
{
    struct list_elem *prev;
    struct list_elem *next;
};

struct list
{
    struct list_elem *head;
};

/* functions for list iteration */
struct list_elem
*list_begin(struct list*);

struct list_elem
*list_end(struct list*);

struct list_elem
*list_next(struct list*);

/* functions for list modification */
void
list_insert_head(struct list*, struct list_elem*);

void
list_insert_tail(struct list*, struct list_elem*);

void
list_remove(struct list_elem*);

#endif
