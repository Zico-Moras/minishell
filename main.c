#include "includes/minishell.h"


// loop do minishell
// read - ft_readline
// evaluate - parser
// TODO

void	shell_loop(t_shell *shell, t_token *tokens)
{
	t_ast_node	*ast;
	while (1)
	{
		setup_signals();
		shell->line = ft_readline(">", shell);
		printf("%s\n", shell->line);
		tokens = lexer(shell->line);
		if (NULL == tokens)
		{
			free_shell(shell);
			continue ;
		}
		token_print(tokens);
		ast = parse(tokens);
		ast_print(ast, 0);
		if (ast)
		{
			if (expand_ast(ast, shell)) // Expand the AST
				ast_print(ast, 0);      // Print the expanded AST
			else
				ft_printf("Expansion failed\n");
			ast_free(ast);              // Free the AST after printing
		}
		token_lstclear(&tokens);
	}
}

int	main(int ac, char **av, char **envp)
{

	(void)ac;
	(void)av;
	(void)envp;
	t_shell	shell;
	t_token tokens;

	ft_memset(&shell, 0, sizeof(t_shell));
	ft_memset(&tokens, 0, sizeof(t_token));
	init_shell(envp, &shell);
	//print envp
	//for(int i = 0; shell.envp[i] != NULL; i++)
	//	ft_printf("%s\n", shell.envp[i]);

	shell_loop(&shell, &tokens);
	free_array(shell.envp);

}
