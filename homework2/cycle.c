#include "shell.h"
#include "cmd.h"
#include "util.h"
#include <signal.h>
#include "sighand.h"

char* prompt;
char* cwd;
char* cmd_char;
struct cmd *cmd_tmp;
struct cmd *cmd_head;
struct history *hist_head;
struct history *hist_curr;
extern char* p;
int argc;
char** argv;
int pipe_enabled;
int left;
int right;
int pipefd[2];
int pipetype; /* 1: std; 2:err */

/* The respond cycle of the shell, from prompt to the finish of execution
 * the return value indicates the status of the cycle.
 */ 
int
respond_cycle()
{
    int cmd_count, return_value;
    pipe_enabled = 0;
    left = right = 0;
    printf("%s [%s]> ", prompt, cwd);
    signal(SIGINT, sigint_handler);
    fgets(cmd_char, 255, stdin); 

    /* deals with IPC stuff */
    int position = find_pipe(cmd_char);
    if (position != -1) {
        pipe_enabled = 1;
        char *new1 = (char*)malloc(sizeof(char[255]));
        char *new2 = (char*)malloc(sizeof(char[255]));
        strncpy(new1, cmd_char, position);
        //debug information
        printf("New cmd 1 is : %s\n", new1);
        if (cmd_char[position+1] == '&') {
            strcpy(new2, cmd_char+position+2);
            pipetype = 2;
        }
        else {
            strcpy(new2, cmd_char+position+1);
            pipetype = 1;
        }
        printf("New cmd 2 is : %s\n", new2);
        /* Builds the pipe */
        if (pipe(pipefd) == -1) {
            printf("YuqiShell: pipe: error: %s\n", strerror(errno));
            return OTHER_ERROR;
        }
        /* doing left side of pipe */
        left = 1;
        right = 0;
        strcpy(cmd_char, new1);
        printf("Starting to execute new1\n");
        cmd_count = parse_cmd();
        printf("ended parsing cmd for new1\n");
        if (pipetype == 1) {
            close(1); /* Closing stdout */
            dup(pipefd[1]);  
            close(pipefd[0]);
        }
        else if (pipetype == 2) {
            close(2); /* Closing stderr */
            dup(pipefd[1]);
            close(1); /* Closing stdout */
            dup(pipefd[1]);
            close(pipefd[0]);
        }
        return_value = find_cmd();
        close(pipefd[1]);
        int fid = open("/dev/tty",O_WRONLY);
        close(1);
        dup(fid);
        close(fid);
        printf("done new1\n");
        /* doing right side of pipe */
        left = 0;
        right = 1;
        strcpy(cmd_char, new2);
        printf("Starting to execute new2\n");
        cmd_count = parse_cmd();
        close(0); /* Closing stdin */
        dup(pipefd[0]);
        close(pipefd[1]);
        if (pipe(pipefd) == -1) {
            printf("YuqiShell: pipe: error: %s\n", strerror(errno));
            return OTHER_ERROR;
        }
        return_value = find_cmd();
    }

  /* No IPC needed to consider */
  else {
      cmd_count = parse_cmd();
      /* deal with background jobs */
      bg_checkall();
      if (cmd_count == CMD_EMPTY)
          return CMD_EMPTY;
      else if (cmd_count == EXIT_SHELL)
          return EXIT_SHELL;

      /* Find the command to execute */
      return_value = find_cmd();
      if (return_value == EXIT_SHELL)
          return EXIT_SHELL;
      else if (return_value == SYNTAX_ERROR)
          return SYNTAX_ERROR;
  }
  return NORMAL;
}

/* prepare for next cycle
 * cleaning pointers and allocating new pointers
 * to prevent memory leak or some weird things
 * from happening
 */
void
prepare_for_next_cycle()
{
  history_add(cmd_char);
  int index;
  if (cmd_char) {
    cmd_char = (char*)realloc(cmd_char, sizeof(char[255]));
    memset(cmd_char, 0, 255);
  }
  cmd_tmp = cmd_head;
  struct cmd *next_to_del;
  while (cmd_tmp != NULL) {
    next_to_del = cmd_tmp -> next;
    if (cmd_tmp)
    //There is a weird problem here. Skipping this right now.
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      //free(cmd_tmp);
        cmd_tmp = next_to_del;
  }
  if (cmd_head)
    cmd_head = (struct cmd*)calloc(1, sizeof(struct cmd));
  if (p)
    p = (char*)realloc(p, sizeof(char[128]));
}

/* the history init function*/
void
history_init(void)
{
  hist_head = (struct history*)malloc(sizeof(struct history));
  hist_curr = hist_head;
}

/* the history_add function */
struct history
*history_add(void)
{
  if (hist_head->cmd == NULL) {
    hist_curr = hist_head;
  }
  else {
    struct history *new = (struct history*)malloc(sizeof(struct history));
    hist_curr -> next = new;
    hist_curr = new;
  }
  hist_curr -> next = NULL;
  hist_curr->cmd = (char*)malloc(sizeof(cmd_char));
  strcpy(hist_curr->cmd, cmd_char);
  return hist_curr;
}

/* the parse function for cmds */
int
parse_cmd()
{
  int count = 0;
  /* EOF */
  if (strlen(cmd_char) == 0) {
      printf("Caught EOF!\n");
      return CMD_EMPTY;
  }
  /* empty command */
  if (strcmp(cmd_char, "\n") == 0) {
    return CMD_EMPTY;
  }

  cmd_char[strlen(cmd_char) - 1] = '\0';
  cmd_tmp = cmd_head;
  int index, flag = 0;
  for (index = 0; index < strlen(cmd_char); index++) {
    if (cmd_char[index] == ' ')  {
        flag = 1;
    }
  }
  if (flag == 1) {
      p = strtok(cmd_char, " ");
      count = 0;
      do {
          if (cmd_head -> element == NULL) {
              cmd_head -> element = p;
              count++;
          }
          else {
              cmd_tmp -> next = calloc(1, sizeof(cmd_head));
              cmd_tmp = cmd_tmp -> next;
              cmd_tmp -> element = p;
              cmd_tmp -> next = NULL;
              count++;
          }
      } while (p = strtok(NULL, " ")); 
  }
  else { /* when there is no blank */
      cmd_head->element = cmd_char;
      cmd_head->next = NULL;
      count = 1;
  }

  cmd_tmp = cmd_head;
  argc = count;
  argv = (char**)malloc((count+1)*sizeof(char**));
  for (index = 0; index < argc; index++) {
    argv[index] = (char*)malloc(sizeof(char[128]));
    strcpy(argv[index], cmd_tmp->element);
    cmd_tmp = cmd_tmp -> next;
    if (cmd_tmp == NULL) break;
  }
  return count; 
}

/* find the command to execute */
int
find_cmd()
{
  int return_value = 0;
  /* find_alias */
  strcpy (cmd_head->element, find_alias(cmd_head->element));

  /* exit */
  if (strcmp(cmd_head->element, "exit") == 0) {
      printf("Typed EXIT, exiting the shell.\n");
      return EXIT_SHELL;
  }
  /* which */
  else if (strcmp(cmd_head->element, "which") == 0) {
      printf("YuqiShell: Executing built-in command \"which\"\n");
      if (argc != 2){
          printf("which: Usage: which <file/command>\n");
          return SYNTAX_ERROR;
      }
      return (cmd_which(argc, argv, 1, NULL));
  }
  /* where */
  else if (strcmp(cmd_head->element, "where") == 0) {
      printf("YuqiShell: Executing built-in command \"where\"\n");
      if (argc != 2){
          printf("YuqiShell: where: Usage: where <file/command>\n");
          return SYNTAX_ERROR;
      }
      return (cmd_which(argc, argv, 2, NULL));
  }
  /* pwd */
  else if (strcmp(cmd_head->element, "pwd") == 0) {
      printf("YuqiShell: Executing built-in command \"pwd\"\n");
      if (argc != 1) {
        printf("YuqiShell: pwd: too many arguments! Only need to type \"pwd\"~~\n");
        return SYNTAX_ERROR;
      }
      cmd_pwd();
      return NORMAL;
  }
  /* cd */
  else if (strcmp(cmd_head->element, "cd") == 0) {
    return (cmd_cd(argc, argv));
  }
  /* ls */
  else if (strcmp(cmd_head->element, "ls") == 0) {
    return (cmd_ls(argc, argv));
  }
  /* pid */
  else if (strcmp(cmd_head->element, "pid") == 0) {
    printf("YuqiShell: Executing built-in command \"pid\"\n");
    if (argc != 1) {
      printf("YuqiShell: pid: too many arguments! Only need to type \"pid\"~~\n");
      return SYNTAX_ERROR;
    }
    cmd_pid();
  }
  /* prompt */
  else if (strcmp(cmd_head->element, "prompt") == 0) {
    return cmd_prompt(argc, argv);
  }
  /* printenv */
  else if (strcmp(cmd_head->element, "printenv")==0) {
    return cmd_printenv(argc, argv, 0);
  }
  /* setenv */
  else if (strcmp(cmd_head->element, "setenv") == 0) {
    return cmd_setenv(argc, argv);
  }
  /* alias */
  else if (strcmp(cmd_head->element, "alias") == 0) {
    return cmd_alias(argc, argv);
  }
  /* history */
  else if (strcmp(cmd_head->element, "history") == 0) {
    cmd_history(argc, argv);
  }
  /* kill */
  else if (strcmp(cmd_head->element, "kill") == 0) {
    return cmd_kill(argc, argv);
  }
  /* jobs */
  else if (strcmp(cmd_head->element, "jobs") == 0) {
    bg_show();
  }
  /* fg */
  else if (strcmp(cmd_head->element, "fg") == 0) {
    cmd_fg(argc, argv);
  }
  /* warnload */
  else if (strcmp(cmd_head->element, "warnload") == 0) {
    cmd_warnload(argc, argv);
  }
  /* watchuser */
  else if (strcmp(cmd_head->element, "watchuser") == 0) {
    cmd_watchuser(argc, argv);
  }
  /* watchmail */
  else if (strcmp(cmd_head->element, "watchmail") == 0) {
    cmd_watchmail(argc, argv);
  }
  /* noclobber */
  else if (strcmp(cmd_head->element, "noclobber") == 0) {
    cmd_noclobber(argc, argv);
  }
  else if (check_outer_cmd(argc, argv) == NORMAL) {
    return NORMAL;
  }
  else {
    printf("YuqiShell: Command %s not found!\n", cmd_head->element);
    return SYNTAX_ERROR;
  }

  return NORMAL;
}
