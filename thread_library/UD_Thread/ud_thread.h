/* 
 * thread library function prototypes
 */
#ifndef H_UD_THREAD
#define H_UD_THREAD
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
#endif
