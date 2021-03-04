#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <sys/types.h>
#include "func.c"

int main(void)
{
    init();

    while(1)
    {
        typePrompt();
        readCommand();
        analyseCommand();

        int pid = fork();

        if(pid < 0) // failed to fork a new thread
        {
            printf("\033[31m\033[1m[x] Unable to fork the child, inner error.\033[0m\n");
        }
        else if(pid == 0) // the child thread
        {
            execvp(args[0], args);
            memset(args, 0, sizeof(char*) * args_count);
            break;
        }
        else // the parent thread
        {
            wait(NULL); //waiting for the child to exit
        }
    }
    return 0;
}