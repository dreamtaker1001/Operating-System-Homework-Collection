#include "shell.h"
#include "cmd.h"

char* prompt;
char* cwd;
char* cmd_char;
struct cmd *cmd_tmp;
struct cmd *cmd_head;
extern char* p;
int argc;
char** argv;

/* The respond cycle of the shell, from prompt to the finish of execution
 * the return value indicates the status of the cycle.
 */ 
int
respond_cycle()
{
  int cmd_count, return_value;
  printf("%s [%s]>  ", prompt, cwd);
  fgets(cmd_char, 255, stdin); 
  //debug information
  //printf("[YuqiShell] The last input cmd was %s\n", cmd_char);

  //debug information
  //printf("[YuqiShell] the last command was %d\n", *cmd_char);
  cmd_count = parse_cmd();
  if (cmd_count == CMD_EMPTY)
      return CMD_EMPTY;
  else if (cmd_count == EXIT_SHELL)
      return EXIT_SHELL;

  /* Find the command to execute */
  return_value = find_cmd();
  if (return_value == EXIT_SHELL)
      return EXIT_SHELL;

  return NORMAL;
}

/* the parse function for cmds */
int
parse_cmd()
{
  int count = 0;
  /* EOF is not done!!!
   
  if (*cmd_char == -1) {
    //debug information
    printf("Caught EOF, shell will now exit!\n");
    return EXIT_SHELL;
  }
  */
  if (strcmp(cmd_char, "\n") == 0) {
    //debug information
    //printf("[YuqiShell] an empty command, will do nothing.\n");
    return CMD_EMPTY;
  }

  cmd_tmp = cmd_head;
  int index, flag = 0;
  for (index = 0; index < strlen(cmd_char); index++) {
    if (cmd_char[index] == ' ')  {
        flag = 1;
        //printf("[YuqiShell] There is a blank in the cmd line!\n");
    }
  }
  if (flag == 1) {
      p = strtok(cmd_char, " ");
      count = 0;
      do {
          if (cmd_head -> element == NULL) {
              cmd_head -> element = p;
              count++;
              //debug information
              //printf("[YuqiShell] The %dst input was %s\n", count, p);
          }
          else {
              cmd_tmp -> next = calloc(1, sizeof(cmd_head));
              cmd_tmp = cmd_tmp -> next;
              cmd_tmp -> element = p;
              cmd_tmp -> next = NULL;
              count++;
              //debug information
              //printf("[YuqiShell] The %dst input was %s\n", count-1, p);
          }
      } while (p = strtok(NULL, " ")); 
  // debug information
  //int a;
  //for (a = 1; a <= count; a++) {
    //printf("[YuqiShell] The %dth element is %s!\n", count, cmd_head[count-1].element);
  //}
  }
  else { /* when there is no blank */
      cmd_char[strlen(cmd_char) - 1] = '\0';
      cmd_head->element = cmd_char;
      cmd_head->next = NULL;
      count = 1;
      //debug information
      //for (index = 0; index < strlen(cmd_head->element); index++)
      //    printf("%d  ", cmd_head->element[index]);
      //printf("\n");
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
  //debug information
  //for (index = 0; index < argc; index++) {
    //printf("%s\n", argv[index]);
  //}
  return count; 
}

/* find the command to execute */
int
find_cmd()
{
  if (strcmp(cmd_head->element, "exit") == 0) {
      printf("Typed EXIT, exiting the shell.\n");
      return EXIT_SHELL;
  }
  else if (strcmp(cmd_head->element, "which") == 0)
      cmd_which(argc, argv);

  return NORMAL;
}
