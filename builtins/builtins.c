#include"../includes/minishell.h"

int exec_builtin(t_command *cmd, t_mini *ms)
{
    if (!cmd || !cmd->cmd_name)
        return (1);
    if (ft_strcmp(cmd->cmd_name, "echo") == 0)
        return (builtin_echo(cmd));
    if (ft_strcmp(cmd->cmd_name, "cd") == 0)
        return (builtin_cd(cmd, ms));
    if (ft_strcmp(cmd->cmd_name, "pwd") == 0)
        return (builtin_pwd(cmd));
    if (ft_strcmp(cmd->cmd_name, "export") == 0)
        return (builtin_export(cmd, ms));
    if (ft_strcmp(cmd->cmd_name, "unset") == 0)
        return (builtin_unset(cmd, ms));
    if (ft_strcmp(cmd->cmd_name, "env") == 0)
        return (builtin_env(cmd, ms));
    if (ft_strcmp(cmd->cmd_name, "exit") == 0)
        return (builtin_exit(cmd));
    return (1);
}

pid_t	control_fork()
{
	pid_t	pid;
	
	pid = fork();
	if (-1 == pid)
		perror("minishell: fork");
	return (pid);
}

