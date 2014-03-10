#include "shell.h"
#include "cmd.h"
#include <signal.h>

extern struct pathelement *path_tmp;
extern struct pathelement *path_list;
extern char *cwd;
extern char *prompt;
extern char **env;
extern char **env_tmp;
char *last_dir;


/* cmd_kill() function
 */
int
cmd_kill(int argc, char** argv)
{
  printf("[YuqiShell] Executing built-in command \"kill\"\n");
  if (argc > 3 || argc == 1) {
    printf("[YuqiShell] kill: syntax error!\n\
            [YuqiShell] kill: Usage: kill <pid>, kill -<SIGNAL> <pid>\n");
    return SYNTAX_ERROR;
  }
  if (argc == 2) {
    if (kill(atoi(argv[1]), SIGTERM) == -1) {
      printf("[YuqiShell] kill: error: %s", strerror(errno));
      return OTHER_ERROR;
    }
  }
  else if (argc == 3) {
    if (*argv[1] != '-') {
        printf("[YuqiShell] kill: syntax error!\n\
                [YuqiShell] kill: Usage: kill <pid>, kill -<SIGNAL> <pid>\n");
        return SYNTAX_ERROR;
    }
    else {
      char sig[5];
      int to_kill;
      strcpy(sig, argv[1]+1);
      if (strcmp(argv[2], "pid-of-shell") == 0) 
          to_kill = getpid();
      else
          to_kill = atoi(argv[2]);
      if (kill(to_kill, atoi(sig)) == -1) {
          printf("[YuqiShell] kill: error: %s", strerror(errno));
          return OTHER_ERROR;
      }
      return NORMAL;
    }
  }
}
