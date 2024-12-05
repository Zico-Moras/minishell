#include "../minishell.h"

int	main()
{
	char	*input;

	while (1)
	{
		input = readline("minishell->"); //TODO
		if (!input)
		{
			printf("exit");
			break;
		}
		//process_input(input); TODO
		free(input);
	}
	return (0);
}
