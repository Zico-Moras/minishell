
#include "../includes/minishell.h"

static void remove_var(t_mini *ms, char *var)
{
	int i;
	int j;
	int len;

	len = ft_strlen(var);
	i = 0;
	while (ms->envp[i])
	{
		if (!ft_strncmp(ms->envp[i], var, len) && ms->envp[i][len] == '=')
		{
			free(ms->envp[i]);
			j = i;
			while (ms->envp[j])
			{
				ms->envp[j] = ms->envp[j + 1];
				j++;
			}
			i--;
		}
		i++;
	}
}

int builtin_unset(t_command *cmd, t_mini *ms)
{
	int i;

	i = 1;
	while (cmd->args[i])
	{
		remove_var(ms, cmd->args[i]);
		i++;
	}
	return (0);
}