#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <sys/types.h>
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

void typePrompt(void)
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

    if(uid == 0) // for root, no color
    {
        printf(user_info->pw_name);
        printf("@");
        printf(local_host_name);
        printf(":");
        if(strlen(current_path) > user_path_len)
        {
            memcpy(user_path, current_path, user_path_len);
            user_path[user_path_len] = '\0';
            if(!strcmp(user_path, user_info->pw_dir))
            {
                printf("~");
                printf(current_path + user_path_len);
            }
            else
                printf(current_path);
        }
        else
        {
            printf(current_path);
        }
        printf("# ");
    }
    else
    {
        printf("\033[32m\033[1m");
        printf(user_info->pw_name);
        printf("@");
        printf(local_host_name);
        printf("\033[0m\033[1m");
        printf(":");
        printf("\033[34m");
        if(strlen(current_path) > user_path_len)
        {
            memcpy(user_path, current_path, user_path_len);
            user_path[user_path_len] = '\0';
            if(!strcmp(user_path, user_info->pw_dir))
            {
                printf("~");
                printf(current_path + user_path_len);
            }
            else
                printf(current_path);
        }
        else
        {
            printf(current_path);
        }
        printf("\033[0m");
        printf("$ ");
    }
}

void readCommand(void)
{
    unsigned long long count = 0;
    char ch;
    while((ch = getchar()) != '\n')
    {
        if(count == command_buf_size)
        {
            if(2 * command_buf_size > BUF_MAX) //overflow
            {
                while((ch = getchar()) != '\n')
                    continue;
                break;
            }
            char * new_buf = (char*)malloc(2 * command_buf_size);
            if(!new_buf) //malloc error
            {
                while((ch = getchar()) != '\n')
                    continue;
                break;
            }
            memcpy(new_buf, command_buf, command_buf_size);
            command_buf_size *= 2;
            free(command_buf);
            command_buf = new_buf;
        }
        command_buf[count++] = ch;
    }
    command_buf[count] = '\0';
    historyRecord();
}

void analyseCommand(void)
{
    args_count = 0;
    args[args_count] = strtok(command_buf, " ");
    char * ptr;
    while(ptr = strtok(NULL, " "))
    {
        args_count++;
        args[args_count] = ptr;
        if(args_count + 1 == ARGS_MAX)
            break;
    }
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
        if(args_count > 1)
            puts("cd: too many arguments");
        else
        {
            if(args[1][0] == '~' && args[1][1] == '/')
            {
                char * dir = malloc(strlen(args[1]) + strlen(user_info->pw_dir));
                strcpy(dir, user_info->pw_dir);
                strncat(dir, args[1][1], strlen(args[1]) - 1);
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
        if(args[1] && !strcmp(args[1], "-c"))
        {
            his_count = 0;
            his_full = 0;
            his_start = 0;
            return 1;
        }
        int count = 0;
        if(his_full)
        {
            for(int i = his_start; i < HIS_MAX; i++)
            {
                printf(" %d  ", count++);
                puts(history[i]);
            }
            for(int i = 0; i < his_start;i++)
            {
                printf(" %d  ", count++);
                puts(history[i]);
            }
        }
        else
        {
            for(int i = 0; i < his_count; i++)
            {
                printf(" %d  ", count++);
                puts(history[i]);
            }
        }
        return 1;
    }
    return 0;
}

void historyRecord(void)
{
    history[his_count] = malloc(strlen(command_buf));
    strcpy(history[his_count], command_buf);
    his_count++;
    if(his_full)
    {
        his_start++;
        his_start %= HIS_MAX;
    }
    if(his_count == HIS_MAX)
    {
        his_count = 0;
        his_full = 1;
    }
}
