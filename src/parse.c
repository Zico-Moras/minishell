#include "../minishell.h"

char	*parse()
{
	char 	*input;
	input = readline("minishell->");
	if (NULL == input || 0 == ft_strncmp(input, "exit", 4))
	{
		printf("exit");
		free(input);
		return (NULL);
	}
	return (input);
}
