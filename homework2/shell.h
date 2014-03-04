#ifndef H_SHELL
#define H_SHELL

/*the init function, initializes the basic environment of the shell*/
void 
shell_init();

/* The respond cycle of the shell, from prompt to the finish of execution
 * the return value indicates the status of the cycle.
 */ 
int 
respond_cycle();

/* The before_exit function. Deals with tracking the status of the
 * running shell and reports errors if any
 */
void
before_exit(int);

#endif
