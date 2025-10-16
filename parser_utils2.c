#include "includes/minishell.h"

/**
 * peek_token - Returns current token without advancing
 * @parser: Parser context
 * 
 * Allows looking at the current token without consuming it.
 * 
 * Returns: Pointer to current token, or NULL if at end
 */
t_token	*peek_token(t_parser *parser)
{
	return (parser->current);
}

/**
 * next_token - Advances to next token
 * @parser: Parser context
 * 
 * Moves the current pointer to the next token in the list.
 * Safe to call even if current is NULL.
 * 
 * Returns: Pointer to new current token, or NULL if at end
 */
t_token	*next_token(t_parser *parser)
{
	if (parser->current)
		parser->current = parser->current->next;
	return (parser->current);
}

/**
 * match_token - Checks if current token matches a type
 * @parser: Parser context
 * @type: Token type to check for
 * 
 * Compares the current token's type with the expected type.
 * 
 * Returns: 1 if match, 0 otherwise
 */
int	match_token(t_parser *parser, t_token_type type)
{
	if (!parser->current)
		return (0);
	return (parser->current->type == type);
}

/**
 * parser_error - Sets error state in parser
 * @parser: Parser context
 * @msg: Error message to store
 * 
 * Marks the parser as having an error and stores the message.
 * The message is duplicated, so caller can free original.
 */
void	parser_error(t_parser *parser, char *msg)
{
	parser->error = 1;
	if (msg)
		parser->error_msg = ft_strdup(msg);
	else
		parser->error_msg = NULL;
}

/**
 * create_pipe_node - Creates a pipe node linking two commands
 * @left: Left side command
 * @right: Right side command
 * 
 * Creates a NODE_PIPE node and sets its children.
 * If allocation fails, frees both children to prevent memory leaks.
 * 
 * Returns: Pipe node, or NULL on failure
 */
t_ast_node	*create_pipe_node(t_ast_node *left, t_ast_node *right)
{
	t_ast_node	*pipe_node;

	pipe_node = ast_new_node(NODE_PIPE);
	if (!pipe_node)
	{
		ast_free(left);
		ast_free(right);
		return (NULL);
	}
	pipe_node->left = left;
	pipe_node->right = right;
	return (pipe_node);
}
