#include "includes/minishell.h"

void	*control_malloc(size_t size, t_shell *shell)
{
	void	*buffer;

	buffer = malloc(size);
	if (!buffer)
	{
		perror("minishell : malloc:");
		shell->exit_status = 2;
		return (NULL);
	}
	return (buffer);
}
