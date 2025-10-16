#include "includes/minishell.h"

t_token	*token_new(t_token_type type, char *value, int quoted)
{
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->type = type;
	token->quoted = quoted;
	token->next = NULL;
	if (value)
	{
		token->value = ft_strdup(value);
		if (!token->value)
		{
			free(token);
			return (NULL);
		}
	}
	else
		token->value = NULL;
	return (token);
}

void	token_del(t_token *token)
{
	if (!token)
		return ;
	if (token->value)
		free(token->value);
	free(token);
}

void	token_lstclear(t_token **head)
{
	t_token	*tmp;

	while (*head)
	{
		tmp = (*head)->next;
		token_del(*head);
		*head = tmp;
	}
}

void	token_lstadd_back(t_token **head, t_token *new)
{
	t_token	*tmp;

	if (!new)
		return ;
	if (!*head)
	{
		*head = new;
		return ;
	}
	tmp = *head;
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = new;
}

void	token_print(t_token *token)
{
	const char	*type_str[8];

	type_str[TOKEN_EOF] = "EOF";
	type_str[TOKEN_WORD] = "WORD";
	type_str[TOKEN_VAR] = "VAR";
	type_str[TOKEN_PIPE] = "PIPE";
	type_str[TOKEN_REDIR_IN] = "REDIR_IN";
	type_str[TOKEN_REDIR_OUT] = "REDIR_OUT";
	type_str[TOKEN_REDIR_APPEND] = "REDIR_APPEND";
	type_str[TOKEN_HEREDOC] = "HEREDOC";
	if (!token)
		return ;
	ft_printf("[%s: '%s' quoted=%d] -> ", type_str[token->type],
		token->value ? token->value : "NULL", token->quoted);
	if (token->next)
		token_print(token->next);
	else
		ft_printf("NULL\n");
}
