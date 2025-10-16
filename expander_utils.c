#includes "includes/minishell.h"

char	*mini_getenv(char *var_name, t_shell *shell)
{
	size_t	len;
	int	i;

	len = ft_strlen(var);
	i = 0;
	while (shell->envp[i])
	{
		if (!ft_strncmp(shell->envp[i], var, len) && shell->envp[i][len] == '=')
			return (shell->envp[i] + len + 1);
		i++;
	}
	return (NULL);
}

/**
 * get_env_value - Gets value of environment variable
 * @var_name: Name of variable to look up
 * @shell: Shell context containing environment and exit status
 * 
 * Handles special variables:
 * - "?" → exit status of last command
 * - "$" → process ID (optional)
 * - Others → lookup in environment
 * 
 * Returns: String value, or "" if not found. Caller must free for $? and $$.
 */
char	*get_env_value(char *var_name, t_shell *shell)
{
	char	*value;

	if (!var_name)
		return (ft_strdup(""));
	if (ft_strcmp(var_name, "?") == 0)
		return (ft_itoa(shell->exit_status));
	if (ft_strcmp(var_name, "$") == 0)
		return (ft_itoa(getpid()));
	value = mini_getenv(var_name);
	if (!value)
		return (ft_strdup(""));
	return (value);
}
