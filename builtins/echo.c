#include"../includes/minishell.h"

int builtin_echo(t_command *cmd)
{
    int  i;
    int  n;
    
    n = 1;
    i = 1;
    
    if (cmd->args[0] && !ft_strcmp(cmd->args[i], "-n"))
    {
        n = 0;
        i++;
    }
    while (cmd->args[i])
    {
        ft_putstr_fd(cmd->args[i], 1);
        if (cmd->args[i + 1])
            ft_putchar_fd(' ', 1);
        i++;
    }
    if (n)
        ft_putchar_fd('\n', 1);
    
    return (0);
}