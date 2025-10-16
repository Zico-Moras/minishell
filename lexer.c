#include "includes/minishell.h"

int	create_word_token(t_token **tokens, char *word, int quote_state)
{
	t_token_type	type;

	type = TOKEN_WORD;
	if (word && ft_strchr(word, '$') && quote_state != 1)
		type = TOKEN_VAR;

	token_lstadd_back(tokens, token_new(type, word, quote_state != 0));
	return (1);
}

int	handle_word(t_token **tokens, char *input, int *i, int quote_state)
{
	int		start;
	char	quote_char;
	char	*word;
	int		len;
	start = *i;
	quote_char = (quote_state == 1 ? '\'' : '"');
	if (advance_word_pos(input, i, quote_state, quote_char) == 0)
		return (0);
	len = *i - start;
	if (quote_state != 0)
		(*i)++;
	if (len == 0)
	{
		if (quote_state == 0)
			return (1);
		word = ft_strdup("");
		if (!word)
			return (0);
	}
	else
	{
		word = ft_substr(input, start, len);
		if (!word)
			return (0);
	}
	int ret = create_word_token(tokens, word, quote_state);
	free(word); 
	return (ret);
}

int	process_quote_open(t_token **tokens, char *input, int *i, int *quote_state)
{
	char	open_quote;

	open_quote = input[*i];
	if (open_quote == '\'')
		*quote_state = 1;
	else
		*quote_state = 2;
	(*i)++;
	return (handle_word(tokens, input, i, *quote_state));
}

int	lexer_loop(t_token **tokens, char *input, int *i, int *quote_state)
{
	skip_whitespace(input, i);
	if (!input[*i])
		return (0);
	if (*quote_state == 0 && (input[*i] == '\'' || input[*i] == '"'))
	{
		if (process_quote_open(tokens, input, i, quote_state) == 0)
			return (0);
		*quote_state = 0;
		return (1);
	}
	if (*quote_state == 0 && handle_operators(tokens, input, i))
		return (1);
	if (handle_word(tokens, input, i, *quote_state) == 0)
		return (0);
	*quote_state = 0;
	return (1);
}

t_token	*lexer(char *input)
{
	t_token	*tokens;
	int		i;
	int		quote_state;

	tokens = NULL;
	i = 0;
	quote_state = 0;
	if (!input)
		return (NULL);
	while (input[i])
	{
		if (lexer_loop(&tokens, input, &i, &quote_state) == 0)
			break ;
	}
	if (quote_state != 0)
	{
		char	quote_str[2];
		char	*err_prefix;
		char	*err_full;

		quote_str[0] = (quote_state == 1 ? '\'' : '"');
		quote_str[1] = '\0';
		err_prefix = ft_strjoin("minishell: syntax error: unexpected EOF while looking for matching `", quote_str);
		err_full = ft_strjoin(err_prefix, "`");
		free(err_prefix);
		ft_putstr_fd(err_full, 2);
		ft_putstr_fd("\n", 2);
		free(err_full);
		token_lstclear(&tokens);
		return (NULL);
	}
	token_lstadd_back(&tokens, token_new(TOKEN_EOF, NULL, 0));
	return (tokens);
}
