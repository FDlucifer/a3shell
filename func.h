#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <sys/types.h>

#define BUF_MAX 0x10000
#define ARGS_MAX 0x100

uid_t uid;
int user_path_len;
char local_host_name[0x100];
char user_path[0x100];
char current_path[0x200];
struct passwd * user_info = NULL;

char * command_buf = NULL;
int command_buf_size = 0;

char * args[0x100];
int args_count = 0;

void init(void);
void typePrompt(void);
void readCommand(void);
void analyseCommand(void);
