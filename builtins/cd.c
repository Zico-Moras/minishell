#include"../includes/minishell.h"

int builtin_cd(t_command *cmd, t_mini *ms)
{
    char *path;
    
    if (cmd->arg_count == 1)
    {
        path = mini_getenv("HOME", ms->envp);
        if (!path)
        {
            fprintf(stderr, "minishell: cd: HOME not set\n");
            return (1);
        }
    }
    else if (cmd->arg_count == 2)
    {
        path = cmd->args[1];
    }
    else
    {
        fprintf(stderr, "minishell: cd: too many arguments\n");
        return (1);
    }
    
    if (chdir(path) == -1)
    {
        perror("minishell: cd");
        return (1);
    }
    
    return (0);
}