#include "includes/minishell.h"

char	**init_envp(char **envp, t_shell *shell)
{
	int	i;
	char	**dup_envp;

	i = 0;

	if (!shell || !envp)
	{
		//ft_putstr_fd("minishell: init_envp: invalid input\n", STDERR_FILENO);
		//shell->exit_status = 1;
		return (NULL);
	}
	while (NULL != envp[i])
		i++;
	dup_envp = control_malloc(sizeof(char *) * (i + 1), shell);
	i = 0;
	while (NULL != envp[i])
	{
		dup_envp[i] = ft_strdup(envp[i]);
		if (NULL == dup_envp[i])
		{
			ft_printf("Problema com o malloc\n");
			exit(69);
		}
		i++;
	}
	dup_envp[i] = NULL;
	return (dup_envp);
}

void	init_shell(char **envp, t_shell *shell)
{
	shell->envp = init_envp(envp, shell);
	shell->exit_status = 0;
}
