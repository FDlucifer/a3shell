#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <wait.h>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "func.h"

void init(void)
{
    command_buf = (char*)malloc(0x500);
    if(!command_buf)
    {
        printf("\033[31m\033[1m[x] Unable to initialize the buffer, malloc error.\033[0m\n");
        exit(-1);
    }
    command_buf_size = 0x500;

}

void getTypePrompt(void)
{
    uid = getuid();
    user_info = getpwuid(uid);
    user_path_len = strlen(user_info->pw_dir);

    if(gethostname(local_host_name, 0x100))
    {
        printf("\033[31m\033[1m[x] Unable to get the hostname, inner error.\033[0m\n");
        exit(-1);
    }

    if(!getcwd(current_path, 0x200))
    {
        printf("\033[31m\033[1m[x] Unable to get the current path, inner error.\033[0m\n");
        exit(-1);
    }

    type_prompt[0] = '\001';
    type_prompt[1] = '\0';

    if(uid == 0) // for root, no color
    {
        strcat(type_prompt, user_info->pw_name);
        strcat(type_prompt, "@");
        strcat(type_prompt, local_host_name);
        strcat(type_prompt, ":");
        if(strlen(current_path) >= user_path_len)
        {
            memcpy(user_path, current_path, user_path_len);
            user_path[user_path_len] = '\0';
            if(!strcmp(user_path, user_info->pw_dir))
            {
                strcat(type_prompt, "~");
                strcat(type_prompt, current_path + user_path_len);
            }
            else
                strcat(type_prompt, current_path);
        }
        else
        {
            strcat(type_prompt, current_path);
        }
        strcat(type_prompt, "# \002");
    }
    else
    {
        strcat(type_prompt, "\033[32m\033[1m");
        strcat(type_prompt, user_info->pw_name);
        strcat(type_prompt, "@");
        strcat(type_prompt, local_host_name);
        strcat(type_prompt, "\033[0m\033[1m");
        strcat(type_prompt, ":");
        strcat(type_prompt, "\033[34m");
        if(strlen(current_path) >= user_path_len)
        {
            memcpy(user_path, current_path, user_path_len);
            user_path[user_path_len] = '\0';
            if(!strcmp(user_path, user_info->pw_dir))
            {
                strcat(type_prompt, "~");
                strcat(type_prompt, current_path + user_path_len);
            }
            else
                strcat(type_prompt, current_path);
        }
        else
        {
            strcat(type_prompt, current_path);
        }
        strcat(type_prompt, "\033[0m");
        strcat(type_prompt, "$ \002");
    }
}

int analyseCommand(void)
{
    args_count = 0;
    if(!(command_buf[0]))
        return FLAG_NULL_INPUT;
    add_history(command_buf);
    args[args_count++] = strtok(command_buf, " ");
    char * ptr;
    while(ptr = strtok(NULL, " "))
    {
        args[args_count++] = ptr;
        if(args_count == ARGS_MAX)
            break;
    }
    if(args[args_count - 1][strlen(args[args_count - 1]) - 1] == '&')
        return FLAG_EXECVE_BACKGROUND;
    return FLAG_EXECVE_WAIT;
}

int innerCommand(void)
{
    if(!strcmp(args[0], "exit"))
    {
        puts("Exit the a3shell now, see you again!");
        exit(-1);
    }
    else if(!strcmp(args[0], "cd"))
    {
        if(args_count > 2)
            puts("cd: too many arguments");
        else
        {
            if(args[1][0] == '~')
            {
                char * dir = malloc(strlen(args[1]) + strlen(user_info->pw_dir));
                if(!dir)
                {
                    printf("\033[31m\033[1m[x] Malloc error. Terminate.\033[0m\n");
                    return 1;
                }
                strcpy(dir, user_info->pw_dir);
                strncat(dir, args[1] + 1, strlen(args[1]) - 1);
                chdir(dir);
                free(dir);
                dir = NULL;
            }
            else
                chdir(args[1]);
        }
        return 1;
    }
    else if(!strcmp(args[0], "history"))
    {
        HIST_ENTRY ** his = history_list();
        for(int i = 0; his[i]; i++)
        {
            printf(" %d\t", i);
            puts(his[i]->line);
        }
        return 1;
    }
    return 0;
}


void createChild(int flag)
{
    int pid = fork();

    if(pid < 0) // failed to fork a new thread
        printf("\033[31m\033[1m[x] Unable to fork the child, inner error.\033[0m\n");
    else if(pid == 0) // the child thread
    {
        execvp(args[0], args);
        exit(0);
    }
    else // the parent thread
        if(flag == FLAG_EXECVE_WAIT)
            wait(NULL); //waiting for the child to exit
}