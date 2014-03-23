#include <sys/stat.h>
#include <sys/wait.h>
#include "shell.h"
#include "util.h"
#include "cmd.h"

extern char **env;
extern unsigned int alarm_time;
extern int alarm_enabled;
extern int sid;
pid_t pid_parent, pid_exec, pid_watchdog;

/* check_outer_cmd() function
 * checks whether command is an outer-source command
 */
int
check_outer_cmd(int argc, char** argv)
{
  if (strncmp(argv[0], "/", 1) == 0 ||
      strncmp(argv[0], "./", 2) == 0 ||
      strncmp(argv[0], "../", 3) == 0) {
    argv[0] = get_absolute_path(argv[0]);
    exec_fixed_path(argc, argv);
  }
  else {
    if (cmd_which(argc, argv, 3, argv[0]) != 0) {
      printf("YuqiShell: command not found!\n");
      return OTHER_ERROR;
    }
    exec_fixed_path(argc, argv);
  }
  return NORMAL;
}

/* get_absolute_path() function,
 * do as its name shows...
 */
char
*get_absolute_path(char* given_path)
{
  char *absolute_path, *first_half, *second_half;
  absolute_path = (char*)calloc(1, sizeof(char[255]));
  memset(absolute_path, 0, 255);
  first_half = (char*)calloc(1, sizeof(char[128]));
  memset(first_half, 0, 128);
  second_half = (char*)calloc(1, sizeof(char[127]));
  memset(second_half, 0, 127);
  if (strncmp(given_path, "./", 2) == 0) {
    getcwd(first_half, 128);
    strcpy(second_half, given_path+2);
    //debug information
    //printf("debug: current dir is %s\n", first_half);
    //printf("debug: second part is %s\n", second_half);
  }
  else if (strncmp(given_path, "../", 3) == 0) {
    getcwd(second_half, 128);
    int i = strlen(second_half) - 1;
    while (second_half[i] != '/') 
      i--;
    strncpy(first_half, second_half, i);
    strcpy(second_half, given_path+3);
    //debug information
    //printf("debug: parent dir is %s\n", first_half);
  }
  /* written for watchmail */
  else if (given_path[0] != '/') {
        getcwd(first_half, 128);
        strcpy(second_half, given_path);
  }
  else {
    strcpy(absolute_path, given_path);
    return absolute_path;
  }
  strcpy(absolute_path, first_half);
  strcat(absolute_path, "/");
  strcat(absolute_path, second_half);
  //debug information
  //printf("debug: the converted absolute dir is %s\n", absolute_path);
  return absolute_path;
}

/* exec_fixed_path() function,
 * find fixed path outsource cmds and execute if matched
 */
int
exec_fixed_path(int argc, char** argv)
{
    pid_parent = pid_exec = pid_watchdog = 0;
    char* given_path = argv[0];
    struct stat st;
    pid_parent = getpid();
    pid_t pid;
    int status;
    int bg_enabled = 0;

    /* enabling background execution of out-source cmds. */
    if (strcmp(argv[argc - 1], "&") == 0) 
        bg_enabled = 1;
    /* test whether object path is executable */
    if (access(given_path, X_OK) == -1) {
        printf("YuqiShell: error: %s\n", strerror(errno));
        return OTHER_ERROR;
    }
    stat(given_path, &st);
    if (S_ISDIR(st.st_mode)) {
        printf("YuqiShell: error: %s is a directory instead of a file!\n", given_path);
        return SYNTAX_ERROR;
    }

    /* ready to execute, creating a new process for execution */
    if ((pid=fork()) < 0) {
        printf("YuqiShell: Cannot create new process!\n");
        return OTHER_ERROR;
        printf("\n");
    }
    /* inside the execution process */
    else if (pid == 0) {
        pid_exec = getpid();
        /* Setting sid, start a new session for background execution */
        if (bg_enabled)
            sid = setsid();
        //debug information
        //printf("debug: child process pid=%d running!\n", pid_exec);
        execve(given_path, argv, env); 
        printf("YuqiShell: error: Can't execute %s\n", given_path);
        return OTHER_ERROR;
    }

    pid_exec = pid; /* The father shell got the executing process's pid*/
    if (alarm_enabled == 1) {
        /* Creating watchdog process */
        if ((pid=fork()) < 0) {
            printf("YuqiShell: Cannot create new process!\n");
            return OTHER_ERROR;
            printf("\n");
        }
        /* inside the watchdog process */
        else if (pid == 0) {
            pid_watchdog = getpid();
            //debug information
            //printf("debug: watchdog process pid=%d running!\n", pid_watchdog);

            sleep(alarm_time);
            if (kill(pid_exec, 0) != -1) {
                kill(pid_exec, SIGKILL);
                printf("YuqiShell: Taking too long to execute this command, terminated!\n");
            }
            exit(0);
        }
    }
    /* ready to wait for the process, but has to specify the
     * OPTION value first.
     * When OPTION value is 0, then wait normally;
     * when OPTION value is WNOHANG, then the waitpid function
     * returns immediately.
     */
    int option;
    if (bg_enabled) {
        option = WNOHANG;
        bg_add(pid_exec);
    }
    else
        option = 0;

    /* father process still waiting for pid_exec.
     * Once pid_exec returns or terminated by watchdog,
     * father process can continue execution.
     */
    if ((pid_exec=waitpid(pid_exec, &status, option)) < 0) {
      printf("YuqiShell: error: waitpid error!\n");
      return OTHER_ERROR;
      printf("YuqiShell: returned value %d\n", status);
    }
    
    return NORMAL;
}
