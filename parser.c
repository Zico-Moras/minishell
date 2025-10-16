#include "includes/minishell.h"
/**
 * parse_command - Parses a simple command
 * @parser: Parser context
 * 
 * Parses a single command with its arguments and redirections.
 * Continues until it hits a pipe, EOF, or NULL.
 * 
 * Example: "cat < in.txt file1 file2 > out.txt"
 * Result: NODE_COMMAND with args=["cat","file1","file2"] 
 *         and redirects=[<in.txt, >out.txt]
 * 
 * Returns: AST node representing the command, or NULL on failure
 */
t_ast_node	*parse_command(t_parser *parser)
{
	t_ast_node	*cmd;

	cmd = ast_new_node(NODE_COMMAND);
	if (!cmd)
		return (NULL);
	while (!is_command_end(parser))
	{
		if (!process_token(parser, cmd))
		{
			ast_free(cmd);
			return (NULL);
		}
	}
	return (cmd);
}
/**
 * parse_pipeline - Parses a pipeline of commands
 * @parser: Parser context
 * 
 * Parses commands connected by pipes.
 * Uses left-associativity: "a | b | c" becomes "((a | b) | c)"
 * 
 * Algorithm:
 * 1. Parse first command (left side)
 * 2. While seeing pipes:
 *    a. Consume pipe token
 *    b. Parse next command (right side)
 *    c. Create pipe node linking left and right
 *    d. Set pipe node as new left (for next iteration)
 * 3. Return the final left node (root of pipeline tree)
 * 
 * Example: "cat | grep | wc"
 * Result:      PIPE
 *             /    \
 *          PIPE    wc
 *         /    \
 *       cat   grep
 * 
 * Returns: AST node representing the pipeline, or NULL on failure
 */

t_ast_node	*parse_pipeline(t_parser *parser)
{
	t_ast_node	*left;
	t_ast_node	*right;

	left = parse_command(parser);
	if (!left)
		return (NULL);
	while (parser->current && parser->current->type == TOKEN_PIPE)
	{
		next_token(parser);
		right = parse_command(parser);
		if (!right)
		{
			ast_free(left);
			return (NULL);
		}
		left = create_pipe_node(left, right);
		if (!left)
			return (NULL);
	}
	return (left);
}

/**
 * print_parser_error - Prints parser error message to stderr
 * @parser: Parser context with error information
 * 
 * Prints the error message stored in the parser, then frees it.
 */
void	print_parser_error(t_parser *parser)
{
	if (parser->error_msg)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(parser->error_msg, 2);
		ft_putstr_fd("\n", 2);
		free(parser->error_msg);
		parser->error_msg = NULL;
	}
}

/**
 * parser_init - Initializes parser context
 * @parser: Parser struct to initialize
 * @tokens: Token list to parse
 * 
 * Sets up the parser with the token list and initializes error tracking.
 * The current pointer starts at the first token.
 */
void	parser_init(t_parser *parser, t_token *tokens)
{
	parser->tokens = tokens;
	parser->current = tokens;
	parser->error = 0;
	parser->error_msg = NULL;
}

/**
 * parse - Main parser entry point
 * @tokens: Token list from lexer
 * 
 * Converts a token list into an Abstract Syntax Tree (AST).
 * 
 * Process:
 * 1. Validate syntax (check for basic errors)
 * 2. Initialize parser context
 * 3. Parse the pipeline (builds AST)
 * 4. Check for parser errors
 * 5. Return AST or NULL on error
 * 
 * The returned AST represents the structure of the command line:
 * - Simple command: NODE_COMMAND with args and redirects
 * - Pipeline: NODE_PIPE with left and right children
 * 
 * Example input: "cat < in | grep test > out"
 * Example output:
 *           PIPE
 *          /    \
 *    COMMAND    COMMAND
 *    cat<in     grep>out
 * 
 * Returns: Root of AST, or NULL on syntax/parse error
 */

t_ast_node	*parse(t_token *tokens)
{
	t_parser	parser;
	t_ast_node	*ast;

	if (!validate_syntax(tokens))
		return (NULL);
	parser_init(&parser, tokens);
	ast = parse_pipeline(&parser);
	if (parser.error)
	{
		print_parser_error(&parser);
		ast_free(ast);
		return (NULL);
	}
	return (ast);
}
