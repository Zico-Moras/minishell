#include"../includes/minishell.h"

int builtin_exit(t_command *cmd)
{
    int exit_code = 0;
    
    if (cmd->arg_count > 1)
        exit_code = atoi(cmd->args[1]);
    
    printf("exit\n");
    exit(exit_code);
}