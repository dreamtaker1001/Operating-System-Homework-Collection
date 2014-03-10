#ifndef H_CMD
#define H_CMD
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

/* cmd_which() function, 
 * which finds the target file in env PATH
 * args:
 *   int argc: count of arguments (command arguments),
 *   char** argv: content of all arguments,
 *   int mode: mode of executing function
 *       1: single mode, used for cmd "which",
 *       2: exhausted mode, used for cmd "where",
 *       3: find mode, used for finding 
 *          non built-in commands.
 */
int 
cmd_which(int, char**, int);


/* cmd_pwd() function,
 * whi*ch prints the current working directory
 */
void
cmd_pwd(void);

/* cmd_cd() function,
 * which changes the current directory
 */
int 
cmd_cd(int, char**);

/* cmd_ls() function,
 * which functions the same as the quite common "ls"
 */
int
cmd_ls(int, char**);

/* ls_single() function,
 * which is used by the cmd_ls() function.
 * It only does the ls operation
 * for a single given directory.
 */
int
ls_single(DIR*);

/* cmd_pid() function,
 * this prints the pid of the current shell
 * on the screen.
 */
void
cmd_pid(void);

/* cmd_prompt() function,
 * this changes the prompt of the current shell.
 */
int
cmd_prompt(int, char**);

/* cmd_printenv() function,
 * gets the env variables of the current env
 * if mode = 0, then it is original call
 * if mode = 1, then it is called by setenv.
 */
int
cmd_printenv(int, char**, int);

/* cmd_setenv() function.
 * works the same as the tcsh built-in one.
 */
int
cmd_setenv(int, char**);

/* cmd_alias() function.
 * still works the same as tsch built-in one.
 */
int
cmd_alias(int, char**);

/* alias_new() function,
 * this builds a new element in the
 * alias linked list.
 */
struct alias
*alias_new(char*, char*);

/* find_alias() function,
 * returns the old name for the alias new name.
 * If alias not found, return argv[0] itself.
 */
char
*find_alias(char*);

/* cmd_history() function
 */
void
cmd_history(int, char**);

/* print_history() function
 */
void
print_history(int);

/* cmd_kill() function
 */
int
cmd_kill(int, char**);
#endif
