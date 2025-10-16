#ifndef MINISHELL_H
# define MINISHELL_H

# include "../libft/libft.h"
# include <readline/readline.h> 
# include <readline/history.h> 
# include <signal.h> 
# include <sys/types.h> 
# include <sys/wait.h> 

//			ENUMS.C				//
typedef enum e_token_type
{
	TOKEN_EOF,
	TOKEN_WORD,
	TOKEN_VAR,
	TOKEN_PIPE,
	TOKEN_REDIR_IN,
	TOKEN_REDIR_OUT,
	TOKEN_REDIR_APPEND,
	TOKEN_HEREDOC
}	t_token_type;

typedef enum e_node_type
{
	NODE_COMMAND,      // Simple command: echo hello
	NODE_PIPE,         // Pipeline: cmd1 | cmd2
	NODE_REDIR_IN,     // Input redirect: < file
	NODE_REDIR_OUT,    // Output redirect: > file
	NODE_REDIR_APPEND, // Append redirect: >> file
	NODE_HEREDOC       // Heredoc: << delimiter
}	t_node_type;

//			TOKENS				//

typedef struct s_token
{
	t_token_type	type;
	char			*value;
	int			quoted;
	struct s_token	*next;
}	t_token;

typedef struct s_shell
{
	char	*line;
	char	**envp;
	int	exit_status;

} t_shell;

//			NODES				//



typedef struct s_redir_node
{
	t_node_type			type;       // REDIR_IN, REDIR_OUT, REDIR_APPEND, HEREDOC
	char				*file;      // Filename or heredoc delimiter (unexpanded)
	struct s_redir_node	*next;      // Next redirection in the list
}	t_redir_node;

/*
** AST Node
** The main tree structure representing the parsed command
*/
typedef struct s_ast_node
{
	t_node_type			type;       // Type of this node
	char				**args;     // Command arguments (NULL-terminated array)
	t_redir_node		*redirects; // List of redirections for this command
	struct s_ast_node	*left;      // Left child (for pipes: left command)
	struct s_ast_node	*right;     // Right child (for pipes: right command)
}	t_ast_node;

/*
** Parser Context
** Holds state while parsing tokens into AST
*/
typedef struct s_parser
{
	t_token		*tokens;        // Current token being processed
	t_token		*current;       // Current position in token stream
	int			error;          // Error flag
	char		*error_msg;     // Error message if parsing fails
}	t_parser;



//			main.c			//

void	shell_loop(t_shell *shell, t_token *tokens);

//			input.c			//
char	*ft_readline(char *prompt, t_shell *shell);


//			signals.c			//

void	setup_signals(void);
void	sigint_handler(int sig);

//			init.c				//

void	init_shell(char **envp, t_shell *shell);
char	**init_envp(char **envp, t_shell *shell);

//			utils.c				//

void	*control_malloc(size_t size, t_shell *shell);



//			free.c				//

void	free_shell(t_shell *shell);
void	free_array(char **envp);


//			lexer.c				//

t_token	*lexer(char *input);
int	create_word_token(t_token **tokens, char *word, int quote_state);
int	handle_word(t_token **tokens, char *input, int *i, int quote_state);
int	process_quote_open(t_token **tokens, char *input, int *i, int *quote_state);
int	lexer_loop(t_token **tokens, char *input, int *i, int *quote_state);

//			lexer_utils.c			//

int	handle_pipe(t_token **tokens, int *i);
int	handle_less(t_token **tokens, char *input, int *i);
int	handle_greater(t_token **tokens, char *input, int *i);
int	handle_operators(t_token **tokens, char *input, int *i);
int	advance_word_pos(char *input, int *i, int quote_state, char quote_char);

//			lexer_utils2.c			//

int	ft_isoperator(char c);
void	skip_whitespace(char *input, int *i);



//			lexer_utils_tokens.c		//
t_token	*token_new(t_token_type type, char *value, int quoted);
void	token_del(t_token *token);
void	token_lstclear(t_token **head);
void	token_lstadd_back(t_token **head, t_token *new);
t_token	*lexer(char *input);
void	token_print(t_token *token);

//			parser.c			//
t_ast_node      *parse(t_token *tokens);
t_ast_node      *parse_pipeline(t_parser *parser);
t_ast_node      *parse_command(t_parser *parser);
void	parser_init(t_parser *parser, t_token *tokens);
void	print_parser_error(t_parser *parser);


//			parser_utils.c			//

t_redir_node	*parse_single_redir(t_parser *parser);
int	handle_redirection(t_parser *parser, t_ast_node *cmd);
int	handle_argument(t_parser *parser, t_ast_node *cmd);
int	is_command_end(t_parser *parser);
int	process_token(t_parser *parser, t_ast_node *cmd);

//			parser_utils.c			//

t_token	*peek_token(t_parser *parser);
t_token	*next_token(t_parser *parser);
int	match_token(t_parser *parser, t_token_type type);
void	parser_error(t_parser *parser, char *msg);
t_ast_node	*create_pipe_node(t_ast_node *left, t_ast_node *right);

//			parser_syntax.c			//

int	validate_syntax(t_token *tokens);
int	validate_pipes(t_token *tokens);
int	validate_redirects(t_token *tokens);

//			parse_node_utils.c		//

t_ast_node	*ast_new_node(t_node_type type);
t_redir_node	*redir_new_node(t_node_type type, char *file);
void	free_args(char **args);
void	redir_free(t_redir_node *redir);
void	ast_free(t_ast_node *node);
void	redir_add_back(t_redir_node **redir_list, t_redir_node *new_redir);
int	redir_count(t_redir_node *redir);
int	args_count(char **args);
char	**args_add(char **args, char *new_arg);
char	**args_dup(char **args);
void	print_indent(int depth);
void	print_node_type(t_node_type type);
void	print_redirects(t_redir_node *redir, int depth);
void	ast_print(t_ast_node *node, int depth);
int	is_redir_type(t_node_type type);
int	is_redir_token(t_token_type type);
t_node_type	token_to_node_type(t_token_type token_type);
int	ast_has_pipes(t_ast_node *node);
int	ast_count_pipes(t_ast_node *node);
int	ast_count_commands(t_ast_node *node);










#endif
