#include <stdio.h>
#include "shell.h"

extern char* prompt;

/* The respond cycle of the shell, from prompt to the finish of execution
 * the return value indicates the status of the cycle.
 */ 
int
respond_cycle ()
{
  printf("\n%s %% ", prompt );
  getchar();
  return 0;
}
