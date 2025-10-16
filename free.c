#include "includes/minishell.h"

void	free_shell(t_shell *shell)
{
	if (shell->line)
		free(shell->line);
}

void	free_array(char **envp)
{
	int	i;
	int	j;

	i = 0;
	j = 0;

	while (NULL != envp[i])
		i++;
	while (i < j)
		free(envp[i++]);
	free(envp);
}
