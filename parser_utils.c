#include "includes/minishell.h"

/**
 * parse_single_redir - Parses a single redirection
 * @parser: Parser context
 * 
 * Parses one redirection operator and its filename.
 * Format: < filename  or  > filename  or  << delimiter  or  >> filename
 * 
 * The function:
 * 1. Saves the redirection type (< > << >>)
 * 2. Advances past the operator
 * 3. Gets the filename from next token
 * 4. Creates a redirection node
 * 5. Advances past the filename
 * 
 * Returns: Redirection node, or NULL on failure
 */
t_redir_node *parse_single_redir(t_parser *parser)
{
    t_node_type     redir_type;
    t_redir_node    *redir;
    int             quoted;

    redir_type = token_to_node_type(parser->current->type);
    next_token(parser);
    if (!parser->current || parser->current->type == TOKEN_EOF)
    {
        parser_error(parser, "unexpected token after redirection");
        return (NULL);
    }
    quoted = parser->current->quoted;  // 🆕 ADD THIS LINE
    redir = redir_new_node(redir_type, parser->current->value, quoted);  // 🆕 UPDATE THIS
    if (!redir)
    {
        parser_error(parser, "memory allocation failed");
        return (NULL);
    }
    next_token(parser);
    return (redir);
}
/**
 * handle_redirection - Parses and adds a redirection to command
 * @parser: Parser context
 * @cmd: Command node to add redirection to
 * 
 * Delegates to parse_single_redir() to parse the redirection,
 * then adds it to the command's redirection list.
 * 
 * Returns: 1 on success, 0 on failure
 */
int	handle_redirection(t_parser *parser, t_ast_node *cmd)
{
	t_redir_node	*redir;

	redir = parse_single_redir(parser);
	if (!redir)
		return (0);
	redir_add_back(&cmd->redirects, redir);
	return (1);
}

/**
 * handle_argument - Adds a word/variable to command arguments
 * @parser: Parser context
 * @cmd: Command node to add argument to
 * 
 * Takes the current token's value and appends it to the command's
 * argument array, then advances to the next token.
 * 
 * Returns: 1 on success, 0 on failure
 */
int handle_argument(t_parser *parser, t_ast_node *cmd)
{
    cmd->args = args_add(cmd->args, &cmd->args_quoted,
                         parser->current->value,
                         parser->current->quoted);  // 🆕 UPDATE THIS LINE
    if (!cmd->args)
        return (0);
    next_token(parser);
    return (1);
}
/**
 * is_command_end - Checks if we've reached the end of a command
 * @parser: Parser context
 * 
 * A command ends when we encounter:
 * - NULL (end of token list)
 * - TOKEN_PIPE (command separator)
 * - TOKEN_EOF (end of input)
 * 
 * Returns: 1 if at end, 0 otherwise
 */
int	is_command_end(t_parser *parser)
{
	if (!parser->current)
		return (1);
	if (parser->current->type == TOKEN_PIPE)
		return (1);
	if (parser->current->type == TOKEN_EOF)
		return (1);
	return (0);
}

/**
 * process_token - Processes a single token in command parsing
 * @parser: Parser context
 * @cmd: Command node being built
 * 
 * Routes the current token to the appropriate handler based on type:
 * - Redirections (< > << >>) → handle_redirection()
 * - Words and variables → handle_argument()
 * - Other tokens → error
 * 
 * Returns: 1 on success, 0 on failure
 */
int	process_token(t_parser *parser, t_ast_node *cmd)
{
	if (is_redir_token(parser->current->type))
		return (handle_redirection(parser, cmd));
	else if (parser->current->type == TOKEN_WORD
		|| parser->current->type == TOKEN_VAR)
		return (handle_argument(parser, cmd));
	return (0);
}
