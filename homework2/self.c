#include "shell.h"
#include "cmd.h"

extern struct pathelement *path_tmp;
extern struct pathelement *path_list;
extern struct alias *alias_head;
extern char *cwd;
extern char *prompt;
extern char **env;
extern char **env_tmp;
char *last_dir;

/* cmd_pid() function,
 * this prints the pid of the current shell
 * on the screen.
 */
void
cmd_pid(void)
{
  pid_t curr_pid;
  curr_pid = getpid();
  printf("[YuqiShell] pid: the pid of the current shell is %d!\n", curr_pid);
}

/* cmd_prompt() function,
 * this changes the prompt of the current shell.
 */
int
cmd_prompt(int argc, char** argv)
{
  if (argc > 2) {
    printf("[YuqiShell] prompt: too many arguments!\n\
            [YuqiShell] prompt: Usage: prompt <return>\n\
            [YuqiShell] prompt: prompt <new_prompt> <return>\n");
    return SYNTAX_ERROR;
  }
  else if (argc == 1) {
    printf("  input prompt prefix: ");
    gets(prompt);
  }
  else if (argc == 2) {
    strcpy(prompt, argv[1]);
  }
  return NORMAL;
}

/* cmd_alias() function.
 * still works the same as tsch built-in one.
 */
int
cmd_alias(int argc, char** argv)
{
  printf("[YuqiShell] Executing built-in command \"alias\"\n");
  
}

/* alias_new() function,
 * this builds a new element in the
 * alias linked list.
 */
struct alias
*alias_new(char* new_name, char* old_name)
{
  
}
