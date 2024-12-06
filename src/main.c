#include "../minishell.h"

int	main()
{
	char	*tokens;
	while (1)
	{
		tokens = parse();
		if (!tokens)
			break;
	}
	return (0);
}
