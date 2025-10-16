#include"../includes/minishell.h"

int builtin_env(t_command *cmd, t_mini *ms)
{
    int i = 0;
    
    (void)cmd;
    
    while (ms->envp[i])
        printf("%s\n", ms->envp[i++]);
    
    return (0);
}