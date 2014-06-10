#include "shell.h"
#include "util.h"

extern char *prompt;
extern char *cwd;
extern char *last_dir;
extern char *cmd_char;
extern char *cmd_char_backup;
extern struct cmd *cmd_head;
struct alias *alias_head;
char *p;
char *env_path;
char **env;
char **env_tmp;
unsigned int alarm_time;
int alarm_enabled = 0;
extern int noclobber;
struct pathelement *path_list = NULL;
struct pathelement *path_tmp = NULL;

/*the init function, initializes the basic environment of the shell*/
void
shell_init(void)
{
    prompt = (char*)malloc(sizeof(char[16]));
    cwd = (char*) malloc(sizeof(char[255]));
    last_dir = (char*)malloc(sizeof(char[255])); 
    strcpy(last_dir, "");

    cmd_char = (char*)calloc(1, sizeof(char[255]));
    cmd_char_backup = (char*)calloc(1, sizeof(char[255]));
    cmd_head = (struct cmd*) malloc(sizeof(struct cmd));
    cmd_head->element = NULL;
    cmd_head->next = NULL;

    p = (char*)malloc(sizeof(char[128]));
    printf("*** Welcome to Yuqi's Shell ! ***\n");
    path_list = (struct pathelement*)malloc(sizeof(struct pathelement));
    path_list -> next = NULL;
    path_list -> element = NULL;
    if (getcwd(cwd, 255) == NULL) {
        printf("YuqiShell: getcwd error: %s\n", strerror(errno));
    }
    /* This is just first_step initialization 
     * to prevent myself from forgetting it 
     * as well as my hands are cut as a consequence...
     */
    strcpy(prompt, "user");
    bg_init();

    path_list = get_path();
    print_env_path();
    alias_init();
    history_init();
    noclobber = 0;
}

/* This initializes the alias system */
void
alias_init(void)
{
    alias_head = (struct alias*)calloc(1, sizeof(struct alias));
    alias_head->old_name = NULL;
    alias_head->new_name = NULL;
    alias_head->next = NULL;
}

/* get_path() function, used to get the current PATH variable*/
struct pathelement 
*get_path()
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
    printf("YuqiShell: The PATH variable in the current system is:\n\t");
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
    if (cwd) {
        free(cwd);
        cwd = NULL;
    }
    if (prompt) {
        free(prompt);
        prompt = NULL;
    }
    if (cmd_char) {
        free(cmd_char);
        cmd_char = NULL;
    }
    if (p) {
        free(p);
        p = NULL;
    }
    if (env_path) {
        free(env_path);
        env_path = NULL;
    }
    if (path_list) {
        path_tmp = path_list;
        while (path_tmp) {
          struct pathelement *next_to_del = path_tmp -> next;
          free(path_tmp);
          path_tmp = NULL;
          path_tmp = next_to_del;
        }
    }
}

/* The main function of this shell
*/
int
main(int argc, char** argv, char** envp)
{
    sigignore(SIGTSTP);
    sigignore(SIGTERM);
    if (argc > 2) {
        printf("YuqiShell: too many arguments!\n");
        return -1;
    } else if (argc == 2) {
        alarm_time = atoi(argv[1]);
        alarm_enabled = 1;
    }
    shell_init();
    int status = 0;
    env = envp;
    while (1) {
        status = respond_cycle();
        if (status == EXIT_SHELL)
            break;
        history_add();
        prepare_for_next_cycle();
    }
    before_exit(status);
    fflush(stdout);
    return 0;
}
