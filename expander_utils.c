#include "includes/minishell.h"

/**
 * ft_charjoin - Append a character to a string
 * @s: String to append to (will be freed)
 * @c: Character to append
 * 
 * Creates new string with character appended.
 * Frees the original string.
 * 
 * Returns: New string with c appended, or NULL on error
 */
char	*ft_charjoin(char *s, char c)
{
	char	*result;
	int		len;
	int		i;

	if (!s)
		return (NULL);
	len = ft_strlen(s);
	result = malloc(len + 2);
	if (!result)
	{
		free(s);
		return (NULL);
	}
	i = 0;
	while (s[i])
	{
		result[i] = s[i];
		i++;
	}
	result[i] = c;
	result[i + 1] = '\0';
	free(s);
	return (result);
}

/**
 * get_env_value - Get value of environment variable
 * @name: Variable name (without $)
 * @shell: Shell context with envp
 * 
 * Searches shell->envp for "NAME=value" format.
 * 
 * Returns: Pointer to value (in envp), or NULL if not found
 */
char	*get_env_value(char *name, t_shell *shell)
{
	int		i;
	int		len;
	char	*env_entry;

	if (!name || !shell || !shell->envp)
		return (NULL);
	len = ft_strlen(name);
	i = 0;
	while (shell->envp[i])
	{
		env_entry = shell->envp[i];
		if (ft_strncmp(env_entry, name, len) == 0 
			&& env_entry[len] == '=')
			return (env_entry + len + 1);
		i++;
	}
	return (NULL);
}

/**
 * get_var_name - Extract variable name from $VAR
 * @str: String starting with $ (e.g., "$HOME")
 * @len: Output parameter for name length
 * 
 * Extracts variable name following bash rules:
 * - Must start with letter or underscore
 * - Can contain letters, digits, underscores
 * - Case-sensitive
 * - Special: $? is valid
 * 
 * Examples:
 *   "$HOME"  → name="HOME", len=4
 *   "$?"     → name="?", len=1
 *   "$123"   → NULL (starts with digit)
 *   "$"      → NULL (nothing after $)
 * 
 * Returns: Pointer to name (after $), or NULL if invalid
 */
char	*get_var_name(char *str, int *len)
{
	int	i;

	if (!str || str[0] != '$')
		return (NULL);
	if (str[1] == '?')
	{
		*len = 1;
		return (str + 1);
	}
	if (!str[1])
	{
		*len = 0;
		return (NULL);
	}
	if (!ft_isalpha(str[1]) && str[1] != '_')
	{
		*len = 0;
		return (NULL);
	}
	i = 1;
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	*len = i - 1;
	return (str + 1);
}
