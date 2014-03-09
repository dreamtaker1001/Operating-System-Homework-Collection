#include "shell.h"

extern char *prompt;
extern char *cwd;
extern char *cmd_char;
extern struct cmd *cmd_head;
char *p;
char *env_path;
struct pathelement *path_list = NULL;
struct pathelement *path_tmp = NULL;

/*the init function, initializes the basic environment of the shell*/
void
shell_init(void)
{
  prompt = (char*) malloc(sizeof(char[16]));
  cwd = (char*) malloc(sizeof(char[255]));
  cmd_char = (char*) malloc(sizeof(char[255]));
  cmd_head = (struct cmd*) malloc(sizeof(struct cmd));
  p = (char*) malloc(sizeof(char[128]));
  printf("*** Welcome to Yuqi's Shell ! ***\n");
  path_list = (struct pathelement*) malloc (sizeof(struct pathelement));
  path_list -> next = NULL;
  path_list -> element = NULL;

  /*This is just first_step initialization 
   * to prevent myself from forgetting it 
   * as well as my hands are cut as a consequence...
   */
  strcpy(prompt, "liuyuqi");
  strcpy(cwd, "/");

  path_list = get_path();
  print_env_path();
}

/* get_path() function, used to get the current PATH variable*/
struct pathelement *get_path()
{
  /* get the PATH variable from system,
   * and copies it to env_path (char* pointer)
   */
  p = getenv("PATH");
  env_path = (char*)malloc((strlen(p)+1)*sizeof(char));
  strncpy(env_path, p, strlen(p));
  env_path[strlen(p)] = '\0'; 

  /* Separate env_path, and build the linked list path_list. */
  p = strtok(env_path, ":");
  path_tmp = path_list;
  do {
    if (path_list -> element == NULL)
        path_list -> element = p;
    else {
      path_tmp -> next = calloc (1, sizeof(struct pathelement));
      path_tmp = path_tmp -> next;
      path_tmp -> element = p;
      path_tmp -> next = NULL;
    }
  } while (p = strtok(NULL, ":"));
 
  return path_list;
}

/* This prints the PATH just gotten from system */
void
print_env_path()
{
  printf("[YuqiShell] The PATH variable in the current system is:\n\t");
  path_tmp = path_list;
  while (path_tmp != NULL) {
    printf("%s", path_tmp->element);
    path_tmp = path_tmp -> next;
    if (path_tmp)
      printf(":");
  }
  puts("\n");
}

/* The before_exit function. Deals with tracking the status of the
 * running shell and reports errors if any
 */
void
before_exit(int status)
{
  free(cwd);
  free(prompt);
  free(cmd_char);
  free(p);
  free(env_path);
  free(path_list);
  free(path_tmp);
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
    if (status == EXIT_SHELL)
      break;
  }
  before_exit(status);
  return 0;
}
