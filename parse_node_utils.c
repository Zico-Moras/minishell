#include "includes/minishell.h"

/* ************************************************************************** */
/*                         NODE CREATION FUNCTIONS                            */
/* ************************************************************************** */

/**
 * ast_new_node - Creates a new AST node
 * @type: The type of node (NODE_COMMAND, NODE_PIPE, etc.)
 * 
 * Allocates memory for a new AST node and initializes all fields to NULL/0.
 * 
 * Returns: Pointer to new node, or NULL on allocation failure
 */
t_ast_node	*ast_new_node(t_node_type type)
{
	t_ast_node	*node;

	node = (t_ast_node *)malloc(sizeof(t_ast_node));
	if (!node)
		return (NULL);
	node->type = type;
	node->args = NULL;
	node->args_quoted = NULL;
	node->redirects = NULL;
	node->left = NULL;
	node->right = NULL;
	return (node);
}

/**
 * redir_new_node - Creates a new redirection node
 * @type: Type of redirection (NODE_REDIR_IN, NODE_REDIR_OUT, etc.)
 * @file: Filename for the redirection (will be duplicated)
 * 
 * Allocates memory for a redirection node and duplicates the filename.
 * 
 * Returns: Pointer to new redirection node, or NULL on failure
 */

t_redir_node *redir_new_node(t_node_type type, char *file, int quoted)
{
    t_redir_node *node;
    
    node = malloc(sizeof(t_redir_node));
    if (!node)
        return (NULL);
    node->type = type;
    node->file = ft_strdup(file);
    node->quoted = quoted;  // 🆕 Store quote info
    if (!node->file)
    {
        free(node);
        return (NULL);
    }
    node->next = NULL;
    return (node);
}

/* ************************************************************************** */
/*                         NODE DESTRUCTION FUNCTIONS                         */
/* ************************************************************************** */

/**
 * free_args - Frees a NULL-terminated array of strings
 * @args: Array of strings to free
 * 
 * Frees each string in the array, then frees the array itself.
 */
void	free_args(char **args)
{
	int	i;

	if (!args)
		return ;
	i = 0;
	while (args[i])
	{
		free(args[i]);
		i++;
	}
	free(args);
}

/**
 * redir_free - Frees a linked list of redirection nodes
 * @redir: First node in the redirection list
 * 
 * Iterates through the redirection list and frees each node.
 */
void	redir_free(t_redir_node *redir)
{
	t_redir_node	*tmp;

	while (redir)
	{
		tmp = redir->next;
		if (redir->file)
			free(redir->file);
		free(redir);
		redir = tmp;
	}
}

void	redir_lstclear(t_redir_node **head)
{
	t_redir_node	*current;
	t_redir_node	*next;

	if (!head || !*head)
		return ;
	current = *head;
	while (current)
	{
		next = current->next;
		free(current->file);
		free(current);
		current = next;
	}
	*head = NULL;
}
/**
 * ast_free - Recursively frees an entire AST
 * @node: Root node of the AST to free
 * 
 * Uses post-order traversal (children first, then parent) to free all nodes.
 * This ensures we don't access freed memory when following pointers.
 */
void ast_free(t_ast_node *node)
{
    if (!node)
        return;
    
    // Free args array and quote array
    if (node->args)
    {
        for (int i = 0; node->args[i]; i++)
            free(node->args[i]);
        free(node->args);
    }
    free(node->args_quoted);  // 🆕 Free quote array
    
    // Free redirects
    redir_lstclear(&node->redirects);
    
    // Recursively free children
    ast_free(node->left);
    ast_free(node->right);
    
    free(node);
}
/* ************************************************************************** */
/*                         REDIRECTION UTILITIES                              */
/* ************************************************************************** */

/**
 * redir_add_back - Adds a redirection node to the end of the list
 * @redir_list: Pointer to the first node in the list
 * @new_redir: New redirection node to add
 * 
 * If the list is empty, sets new_redir as the first node.
 * Otherwise, traverses to the end and appends new_redir.
 */
void	redir_add_back(t_redir_node **redir_list, t_redir_node *new_redir)
{
	t_redir_node	*current;

	if (!redir_list || !new_redir)
		return ;
	if (!*redir_list)
	{
		*redir_list = new_redir;
		return ;
	}
	current = *redir_list;
	while (current->next)
		current = current->next;
	current->next = new_redir;
}

/**
 * redir_count - Counts the number of redirections in a list
 * @redir: First node in the redirection list
 * 
 * Returns: Number of redirection nodes in the list
 */
int	redir_count(t_redir_node *redir)
{
	int	count;

	count = 0;
	while (redir)
	{
		count++;
		redir = redir->next;
	}
	return (count);
}

/* ************************************************************************** */
/*                         ARGS ARRAY UTILITIES                               */
/* ************************************************************************** */

/**
 * args_count - Counts the number of arguments in a NULL-terminated array
 * @args: Array of strings
 * 
 * Returns: Number of strings in the array (not counting NULL terminator)
 */
int	args_count(char **args)
{
	int	count;

	if (!args)
		return (0);
	count = 0;
	while (args[count])
		count++;
	return (count);
}

/**
 * args_add - Adds a new argument to an args array
 * @args: Pointer to existing args array (may be NULL)
 * @new_arg: New argument string to add (will be duplicated)
 * 
 * Reallocates the array to fit the new argument and adds it at the end.
 * The array remains NULL-terminated.
 * 
 * Returns: New args array, or NULL on failure
 */
char **args_add(char **args, int **args_quoted, char *new_arg, int quoted)
{
    int     len;
    char    **new_args;
    int     *new_quoted;
    int     i;

    len = 0;
    while (args && args[len])
        len++;
    new_args = malloc(sizeof(char *) * (len + 2));
    new_quoted = malloc(sizeof(int) * (len + 2));
    if (!new_args || !new_quoted)
        return (free(new_args), free(new_quoted), NULL);
    i = -1;
    while (++i < len)
    {
        new_args[i] = args[i];
        new_quoted[i] = (*args_quoted)[i];
    }
    new_args[len] = ft_strdup(new_arg);
    new_quoted[len] = quoted;
    new_args[len + 1] = NULL;
    new_quoted[len + 1] = 0;
    free(args);
    free(*args_quoted);
    *args_quoted = new_quoted;
    return (new_args);
}
/**
 * args_dup - Duplicates an entire args array
 * @args: Array to duplicate
 * 
 * Creates a deep copy of the args array, duplicating each string.
 * 
 * Returns: New duplicated array, or NULL on failure
 */
char	**args_dup(char **args)
{
	char	**dup;
	int		count;
	int		i;

	if (!args)
		return (NULL);
	count = args_count(args);
	dup = (char **)malloc(sizeof(char *) * (count + 1));
	if (!dup)
		return (NULL);
	i = 0;
	while (i < count)
	{
		dup[i] = ft_strdup(args[i]);
		if (!dup[i])
		{
			free_args(dup);
			return (NULL);
		}
		i++;
	}
	dup[i] = NULL;
	return (dup);
}

/* ************************************************************************** */
/*                         DEBUG/PRINT FUNCTIONS                              */
/* ************************************************************************** */

/**
 * print_indent - Prints indentation for tree visualization
 * @depth: Number of indentation levels
 */

void print_indent(int depth)
{
    int i;

    i = 0;
    while (i < depth)
    {
        ft_printf("  ");  // Two spaces per depth level
        i++;
    }
}

/**
 * print_node_type - Prints the type of an AST node
 * @type: Node type to print
 */
void print_node_type(t_node_type type)
{
    if (type == NODE_COMMAND)
        ft_printf("COMMAND");
    else if (type == NODE_PIPE)
        ft_printf("PIPE");
    else if (type == NODE_REDIR_IN)
        ft_printf("REDIR_IN");
    else if (type == NODE_REDIR_OUT)
        ft_printf("REDIR_OUT");
    else if (type == NODE_REDIR_APPEND)
        ft_printf("REDIR_APPEND");
    else if (type == NODE_HEREDOC)
        ft_printf("HEREDOC");
    else
        ft_printf("UNKNOWN");
}
// Helper to get quoted description
static const char *get_quoted_desc(int quoted)
{
    if (quoted == 1)
        return "quoted=1 (single quotes)";
    else if (quoted == 2)
        return "quoted=2 (double quotes)";
    else
        return "quoted=0 (unquoted)";
}
/**
 * print_redirects - Prints all redirections for a command
 * @redir: First redirection node
 * @depth: Indentation depth
 */
void print_redirects(t_redir_node *redir, int depth)
{
    t_redir_node *current;

    current = redir;
    print_indent(depth);
    ft_printf("Redirects:\n");
    if (!current)
    {
        print_indent(depth + 1);
        ft_printf("(none)\n");
        return;
    }
    while (current)
    {
        print_indent(depth + 1);
        print_node_type(current->type);
        ft_printf(": '%s' (%s)\n", current->file ? current->file : "NULL", get_quoted_desc(current->quoted));
        current = current->next;
    }
}
/**
 * ast_print - Prints the AST in a tree format (for debugging)
 * @node: Root of the AST to print
 * @depth: Current depth (use 0 for root)
 * 
 * Recursively prints the AST structure with indentation showing depth.
 * Shows node types, arguments, and redirections.
 */

// Main AST print function (recursive)
void ast_print(t_ast_node *node, int depth)
{
    int i;

    if (!node)
    {
        print_indent(depth);
        ft_printf("NULL\n");
        return;
    }
    print_indent(depth);
    print_node_type(node->type);
    ft_printf(":\n");
    if (node->type == NODE_COMMAND)
    {
        print_indent(depth + 1);
        ft_printf("Args:\n");
        if (!node->args)
        {
            print_indent(depth + 2);
            ft_printf("(none)\n");
        }
        else
        {
            i = 0;
            while (node->args[i])
            {
                print_indent(depth + 2);
                ft_printf("'%s' (%s)\n", node->args[i], get_quoted_desc(node->args_quoted ? node->args_quoted[i] : 0));
                i++;
            }
        }
        print_redirects(node->redirects, depth + 1);
    }
    else if (node->type == NODE_PIPE)
    {
        print_indent(depth + 1);
        ft_printf("Left:\n");
        ast_print(node->left, depth + 2);
        print_indent(depth + 1);
        ft_printf("Right:\n");
        ast_print(node->right, depth + 2);
    }
}

/* ************************************************************************** */
/*                         TYPE CHECKING UTILITIES                            */
/* ************************************************************************** */

/**
 * is_redir_type - Checks if a node type is a redirection
 * @type: Node type to check
 * 
 * Returns: 1 if type is a redirection, 0 otherwise
 */
int	is_redir_type(t_node_type type)
{
	return (type == NODE_REDIR_IN || type == NODE_REDIR_OUT
		|| type == NODE_REDIR_APPEND || type == NODE_HEREDOC);
}

/**
 * is_redir_token - Checks if a token type is a redirection operator
 * @type: Token type to check
 * 
 * Returns: 1 if token is a redirection operator, 0 otherwise
 */
int	is_redir_token(t_token_type type)
{
	return (type == TOKEN_REDIR_IN || type == TOKEN_REDIR_OUT
		|| type == TOKEN_REDIR_APPEND || type == TOKEN_HEREDOC);
}

/**
 * token_to_node_type - Converts token type to node type
 * @token_type: Token type to convert
 * 
 * Used when converting redirection tokens to redirection nodes.
 * 
 * Returns: Corresponding node type, or NODE_COMMAND if not a redirection
 */
t_node_type	token_to_node_type(t_token_type token_type)
{
	if (token_type == TOKEN_REDIR_IN)
		return (NODE_REDIR_IN);
	if (token_type == TOKEN_REDIR_OUT)
		return (NODE_REDIR_OUT);
	if (token_type == TOKEN_REDIR_APPEND)
		return (NODE_REDIR_APPEND);
	if (token_type == TOKEN_HEREDOC)
		return (NODE_HEREDOC);
	return (NODE_COMMAND);
}

/* ************************************************************************** */
/*                         AST QUERY FUNCTIONS                                */
/* ************************************************************************** */

/**
 * ast_has_pipes - Checks if AST contains any pipe nodes
 * @node: Root of AST to check
 * 
 * Returns: 1 if AST contains pipes, 0 otherwise
 */
int	ast_has_pipes(t_ast_node *node)
{
	if (!node)
		return (0);
	if (node->type == NODE_PIPE)
		return (1);
	return (ast_has_pipes(node->left) || ast_has_pipes(node->right));
}

/**
 * ast_count_pipes - Counts the number of pipes in the AST
 * @node: Root of AST to count
 * 
 * Returns: Number of pipe nodes in the tree
 */
int	ast_count_pipes(t_ast_node *node)
{
	if (!node)
		return (0);
	if (node->type == NODE_PIPE)
		return (1 + ast_count_pipes(node->left) + ast_count_pipes(node->right));
	return (ast_count_pipes(node->left) + ast_count_pipes(node->right));
}

/**
 * ast_count_commands - Counts the number of command nodes in the AST
 * @node: Root of AST to count
 * 
 * Returns: Number of command nodes in the tree
 */
int	ast_count_commands(t_ast_node *node)
{
	if (!node)
		return (0);
	if (node->type == NODE_COMMAND)
		return (1);
	return (ast_count_commands(node->left) + ast_count_commands(node->right));
}
