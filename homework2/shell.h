#ifndef H_SHELL
#define H_SHELL

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define CMD_EMPTY -1
#define NORMAL 0
#define EXIT_SHELL -2

/* the PATH data structure */
struct pathelement
{
  char *element;
  struct pathelement *next;
};

/* the cmd data strucure */
struct cmd
{
  char *element;
  struct cmd *next;
};

/* the parse function for cmds,
 * the return value is "argc"
 */
int
parse_cmd();

/* find the command to execute */
int
find_cmd();

/*the init function, initializes the basic environment of the shell*/
void 
shell_init();

/* This prints the PATH just gotten from system */
void
print_env_path();

/* get_path() function, used to get the current PATH variable*/
struct pathelement *get_path();

/* The respond cycle of the shell, 
 * from prompt to the finish of execution.
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
