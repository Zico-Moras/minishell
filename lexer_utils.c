#include "includes/minishell.h"

int	handle_pipe(t_token **tokens, int *i)
{
	char	*op_value;

	op_value = ft_strdup("|");
	if (!op_value)
		return (0);
	token_lstadd_back(tokens, token_new(TOKEN_PIPE, op_value, 0));
	(*i)++;
	return (1);
}

int	handle_less(t_token **tokens, char *input, int *i)
{
	char	*op_value;

	if (input[*i + 1] == '<')
	{
		op_value = ft_strdup("<<");
		if (!op_value)
			return (0);
		token_lstadd_back(tokens, token_new(TOKEN_HEREDOC, op_value, 0));
		(*i) += 2;
	}
	else
	{
		op_value = ft_strdup("<");
		if (!op_value)
			return (0);
		token_lstadd_back(tokens, token_new(TOKEN_REDIR_IN, op_value, 0));
		(*i)++;
	}
	return (1);
}

int	handle_greater(t_token **tokens, char *input, int *i)
{
	char	*op_value;

	if (input[*i + 1] == '>')
	{
		op_value = ft_strdup(">>");
		if (!op_value)
			return (0);
		token_lstadd_back(tokens, token_new(TOKEN_REDIR_APPEND, op_value, 0));
		(*i) += 2;
	}
	else
	{
		op_value = ft_strdup(">");
		if (!op_value)
			return (0);
		token_lstadd_back(tokens, token_new(TOKEN_REDIR_OUT, op_value, 0));
		(*i)++;
	}
	return (1);
}

int	handle_operators(t_token **tokens, char *input, int *i)
{
	if (input[*i] == '|')
		return (handle_pipe(tokens, i));
	if (input[*i] == '<')
		return (handle_less(tokens, input, i));
	if (input[*i] == '>')
		return (handle_greater(tokens, input, i));
	return (0);
}

int	advance_word_pos(char *input, int *i, int quote_state, char quote_char)
{
	while (input[*i] && (quote_state != 0
			|| (!ft_isspace((unsigned char)input[*i]) && !ft_isoperator
				(input[*i]) && input[*i] != '\'' && input[*i] != '"')))
	{
		if (quote_state != 0 && input[*i] == quote_char)
			return (1);
		(*i)++;
	}
	if (quote_state != 0)
		return (0);
	return (1);
}
