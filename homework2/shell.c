#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"

char* prompt;


/*the init function, initializes the basic environment of the shell*/
void
shell_init(void)
{
  prompt = (char*) malloc (sizeof (char[32]));
  strcpy(prompt, "liuyuqi");


  printf("*** Welcome to Yuqi's Shell ! ***\n");
}

/* The before_exit function. Deals with tracking the status of the
 * running shell and reports errors if any
 */
void
before_exit(int status)
{

  free(prompt);
}

/* The main function of this shell
 */
int
main(void)
{
  shell_init();
  int status = 0;
  while (1) {
    status = respond_cycle();
  }
  before_exit(status);
  return 0;
}
