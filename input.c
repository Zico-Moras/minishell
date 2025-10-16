#include "includes/minishell.h"

// receve input atraves de readline e da return
// da handle ao sinal Ctrl-kd (EOF) printando exit em stdout e dando exit(0)
char	*ft_readline(char *prompt, t_shell *shell)
{
	char	*line;

	line = readline(prompt);
	if (!line)
	{
		ft_printf("exit\n");
		free_array(shell->envp);
		exit(0);
	}
	else if (line && ft_strcmp(line, "\n") != 0)
		add_history(line);
	return (line);
}

