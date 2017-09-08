#include "zengjf.h"

// command
#define CMD_BUF_SIZE 512

char cmd_buf[CMD_BUF_SIZE] = {0};
int cmd_buf_count = 0;

void get_cmd_parser_char(void) 
{
    char ch = getchar();
    
    if (ch == 127 || ch == 8) {
        --cmd_buf_count;
        
        if (cmd_buf_count < 0)
            cmd_buf_count = 0;

        if (cmd_buf_count > CMD_BUF_SIZE - 1)
            cmd_buf_count = CMD_BUF_SIZE - 1;
        
        cmd_buf[cmd_buf_count] = 0;
        printf("\033[1K\rAplexOS # %s", cmd_buf);

        return;
    } 
    else if (ch == '\r')
    {     
        if (strlen(cmd_buf) != 0) 
        {
            shell_process(cmd_buf);
            printf("\r\n");
        }
        
        printf("AplexOS # ");

        memset(cmd_buf, 0, strlen(cmd_buf));
        cmd_buf_count = 0;
        
        return;
    }
    
    cmd_buf[cmd_buf_count++] = ch;
}

char* welcome_msg =
    "\r\n\r\nHardware Auto Detect System v0.0.1 (" __DATE__ ")\r\n"
    "\r\n               ---- Designed By AplexOS Team \r\n\r\n";

int shell_cmd_list(char *args);

shell_cmds microcli_shell_cmds =
{
    .count = 1,
    .cmds  = {
        {
            .cmd     = "list",
            .desc    = "List available",
            .func    = shell_cmd_list,
        },
    },
};

int shell_cmd_list(char *args) 
{
    printf("%s", welcome_msg);
    return 0;
}

int shell_process(char *cmd_line) 
{
    int i = 0;
    for (i = 0; i < microcli_shell_cmds.count; i++) 
    {
        if (strncmp(microcli_shell_cmds.cmds[i].cmd, cmd_line, strlen(microcli_shell_cmds.cmds[i].cmd)) == 0) 
        {
            if (cmd_line[strlen(microcli_shell_cmds.cmds[i].cmd)] == ' ') 
            {
                microcli_shell_cmds.cmds[i].func(&cmd_line[strlen(microcli_shell_cmds.cmds[i].cmd) + 1]);
            } else if(strlen(cmd_line) == strlen(microcli_shell_cmds.cmds[i].cmd)) {
                microcli_shell_cmds.cmds[i].func(NULL);
            }
            
            return 0;
        }
    }
    
    printf("unsupport command.");
    return -1;
}

