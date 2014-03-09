#include "shell.h"
#include "cmd.h"

/* which() function, 
 * which finds the target file in env PATH
 */
void 
cmd_which(int argc, char **argv)
{
   printf("[YuqiShell] Executing built-in command \"which\"\n");
   if (argc == 1){
     printf("which: Uses: which <file/command>\n");
   }

   return; 
}
