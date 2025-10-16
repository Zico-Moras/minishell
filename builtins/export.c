
#include "../includes/minishell.h"

static int find_var(char **envp, char *name)
{
	int i;
	int len;

	len = 0;
	i = 0;
	while (name[len] && name[len] != '=')
		len++;
	while (envp[i])
	{
		if (!ft_strncmp(envp[i], name, len) && envp[i][len] == '=')
			return (i);
		i++;
	}
	return (-1);
}

static int add_var(t_mini *ms, char *arg)
{
	int i;
	char **new_envp;

	i = 0;
	while (ms->envp[i])
		i++;
	new_envp = (char **)malloc(sizeof(char *) * (i + 2));
	if (!new_envp)
		return (1);
	i = 0;
	while (ms->envp[i])
	{
		new_envp[i] = ms->envp[i];
		i++;
	}
	new_envp[i] = ft_strdup(arg);
	new_envp[i + 1] = NULL;
	free(ms->envp);
	ms->envp = new_envp;
	return (0);
}

int builtin_export(t_command *cmd, t_mini *ms)
{
	int i;
	int pos;

	i = 1;
	if (cmd->arg_count < 2)
	{
		i = 0;
		while (ms->envp[i])
		{
			ft_putstr_fd("declare -x ", 1);
			ft_putstr_fd(ms->envp[i], 1);
			ft_putchar_fd('\n', 1);
			i++;
		}
		return (0);
	}
	while (cmd->args[i])
	{
		if (ft_strchr(cmd->args[i], '='))
		{
			pos = find_var(ms->envp, cmd->args[i]);
			if (pos >= 0)
			{
				free(ms->envp[pos]);
				ms->envp[pos] = ft_strdup(cmd->args[i]);
			}
			else
				add_var(ms, cmd->args[i]);
		}
		i++;
	}
	return (0);
}