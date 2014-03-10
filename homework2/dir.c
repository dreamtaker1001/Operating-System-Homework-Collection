#include "shell.h"
#include "cmd.h"

extern struct pathelement *path_tmp;
extern struct pathelement *path_list;
extern char *cwd;
extern char *prompt;
extern char **env;
extern char **env_tmp;
char *last_dir;

/* cmd_cd() function,
 * which changes the current directory
 */
int 
cmd_cd(int argc, char** argv)
{
  int fd, reason;      
  char *new_dir = NULL;
  printf("[YuqiShell] Executing built-in command \"cd\"\n");
  if (argc > 2) {
    printf("[YuqiShell] cd: too many arguments! \n \
            [YuqiShell] cd: Usage: cd <directory> OR cd OR cd -\n");
    return SYNTAX_ERROR;
  }
  /* cd to home directory */
  if (argc == 1) {
    if ((new_dir=getenv("HOME")) == NULL) {
      printf("[YuqiShell] cd: unable to get home dir!\n");
      return OTHER_ERROR;
    }
  }
  else if (argc == 2) {
    if (argv[1] == "-") {
      if (strcmp(last_dir, ""))
        new_dir = last_dir;
      else {
        printf("[YuqiShell] cd: error: do not have last dir yet!\n");
        return OTHER_ERROR;
      }
    }
    else new_dir = argv[1];
  }

  /* Changes the dir */
  if (chdir(new_dir) != 0) {
    printf("[YuqiShell] cd: error changing dir, %s\n", strerror(errno));
    return OTHER_ERROR;
  }
  else close(fd);
  strcpy(cwd, new_dir);
  return NORMAL;
}


/* cmd_ls() function,
 * which functions the same as the quite common "ls"
 */
int
cmd_ls(int argc, char** argv)
{
  printf("[YuqiShell] Executing built-in command \"ls\"\n");
  int return_value;
  int wildcard = 0;
  DIR* curr_dir;
  char* dir_to_list;
  dir_to_list = (char*)calloc(1, sizeof(char[255]));
  /* in case only typing "ls" */
  if (argc == 1) {
    strcpy(dir_to_list, cwd);
  }
  /* in case of typing "ls dir" */
  else if (argc == 2) {
    strcpy(dir_to_list, argv[1]);
  }

  curr_dir = opendir(dir_to_list);
  if (curr_dir == NULL) {
      if (errno == EACCES) {
          printf("[YuqiShell] ls: error: permission dinied!\n");
          return OTHER_ERROR;
      }
      else if (errno == ENOENT) {
          printf("[YuqiShell] ls: error: Directory \"%s\" doesn't exist!\n", dir_to_list);
          return OTHER_ERROR;
      }
      else if (errno == ENOTDIR) {
          printf("[YuqiShell] ls: error: \"%s\" is not a directory!\n", dir_to_list);
          return OTHER_ERROR;
      }
  }
  
  /* ready to output */
  if (argc <= 2 && wildcard == 0) {
    return_value = ls_single(curr_dir);
  }
  /* This is not done yet!! */
  else {
      //wildcard(argc, argv);
  }

  return NORMAL;
}

/* ls_single() function,
 * which is used by the cmd_ls() function.
 * It only does the ls operation
 * for a single given directory.
 */
int
ls_single(DIR* dir_to_list)
{
  struct dirent *ptr;
  while ((ptr=readdir(dir_to_list)) != NULL) {
    printf("%s\n", ptr->d_name);
  }
  return NORMAL;
}

/* pwd() function,
 * which prints the current working directory
 */
void
cmd_pwd()
{
  printf("%s\n", cwd);
}



