#include "includes/minishell.h"

int	ft_isoperator(char c)
{
	return (c == '|' || c == '<' || c == '>');
}

void	skip_whitespace(char *input, int *i)
{
	while (input[*i] && ft_isspace((unsigned char)input[*i]))
		(*i)++;
}
