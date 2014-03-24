#include "shell.h"
#include "cmd.h"
#include "util.h"

extern int pipe_enabled;

/* find whether a pipe exists in a command */
int
find_pipe(char *cmd_char)
{
    int i;
    for (i = 0; i < strlen(cmd_char); i++) {
        if (cmd_char[i] == '|')
            return i;
    }
    return -1;
}

