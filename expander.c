#include "includes/minishell.h"

/**
 * get_expanded_value - Get expanded value for a variable
 * @var_name: Variable name (e.g., "HOME" or "?")
 * @name_len: Length of variable name
 * @shell: Shell context
 *
 * Returns: Allocated string with value, or NULL on error
 */
static char	*get_expanded_value(char *var_name, int name_len, t_shell *shell)
{
	char	*value;
	char	*var_name_dup;

	if (!var_name || !shell)
		return (NULL);
	if (name_len == 1 && var_name[0] == '?')
		return (ft_itoa(shell->exit_status));
	var_name_dup = ft_substr(var_name, 0, name_len);
	if (!var_name_dup)
		return (NULL);
	value = get_env_value(var_name_dup, shell);
	free(var_name_dup);
	if (value)
		return (ft_strdup(value));
	return (ft_strdup("")); // Return empty string for undefined variables
}

/**
 * append_expansion - Expand one variable and append to result
 * @result: Current result string
 * @str: Original string (positioned at $)
 * @shell: Shell context
 * @i: Current position (updated to skip past variable)
 *
 * Returns: New result string with variable expanded, or NULL on error
 */
static char	*append_expansion(char *result, char *str, t_shell *shell, int *i)
{
	char	*var_name;
	int		name_len;
	char	*value;
	char	*temp;

	if (!result || !str || !shell)
		return (NULL);
	var_name = get_var_name(str + *i, &name_len);
	if (!var_name)
	{
		temp = ft_charjoin(result, '$');
		(*i)++;
		return (temp);
	}
	value = get_expanded_value(var_name, name_len, shell);
	if (!value)
	{
		free(result);
		return (NULL);
	}
	temp = ft_strjoin(result, value);
	free(result);
	free(value);
	if (!temp)
		return (NULL);
	*i += name_len + 1;
	return (temp);
}

/**
 * expand_string - Expand variables in a string
 * @str: String to expand (may contain $VAR)
 * @quoted: Quote status (0=none, 1=single, 2=double)
 * @shell: Shell context
 *
 * Returns: Allocated string with expansions done, or NULL on error
 */
char	*expand_string(char *str, int quoted, t_shell *shell)
{
	char	*result;
	int		i;
	char	*temp;

	if (!str || !shell)
		return (NULL);
	if (quoted == 1)
		return (ft_strdup(str));
	result = ft_strdup("");
	if (!result)
		return (NULL);
	i = 0;
	while (str[i])
	{
		if (str[i] == '$' && str[i + 1] && quoted != 1)
			result = append_expansion(result, str, shell, &i);
		else
		{
			temp = ft_charjoin(result, str[i]);
			if (!temp)
			{
				free(result);
				return (NULL);
			}
			result = temp;
			i++;
		}
		if (!result) // Check for failure in append_expansion
			return (NULL);
	}
	return (result);
}

/**
 * expand_ast - Recursively expand entire AST
 * @ast: Root of AST tree
 * @shell: Shell context
 *
 * Returns: 1 on success, 0 on error
 */
int	expand_ast(t_ast_node *ast, t_shell *shell)
{
	t_redir_node	*redir;
	char			*expanded;
	int				i;

	if (!ast || !shell)
		return (0);
	if (ast->type == NODE_COMMAND)
	{
		i = 0;
		while (ast->args && ast->args[i])
		{
			if (ast->args_quoted[i] != 1)
			{
				expanded = expand_string(ast->args[i], ast->args_quoted[i], shell);
				if (!expanded)
					return (0);
				free(ast->args[i]);
				ast->args[i] = expanded;
			}
			i++;
		}
		redir = ast->redirects;
		while (redir)
		{
			if (redir->quoted != 1)
			{
				expanded = expand_string(redir->file, redir->quoted, shell);
				if (!expanded)
					return (0);
				free(redir->file);
				redir->file = expanded;
			}
			redir = redir->next;
		}
	}
	else if (ast->type == NODE_PIPE)
	{
		if (!expand_ast(ast->left, shell) || !expand_ast(ast->right, shell))
			return (0);
	}
	return (1);
}
