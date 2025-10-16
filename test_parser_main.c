#include "includes/minishell.h"
#include <stdio.h>

// ANSI Colors
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"
#define RESET   "\033[0m"

typedef struct s_parser_test {
	char	*input;
	int		expect_error;
	char	*desc;
} t_parser_test;

static void	print_separator(void)
{
	ft_printf("\n%s%s", CYAN, "═══════════════════════════════════════════════════════════════════\n");
	ft_printf("%s", RESET);
}

static int	run_parser_test(t_parser_test *test, int test_num)
{
	t_token		*tokens;
	t_ast_node	*ast;
	int			passed;

	ft_printf("\n%s%s=== Test %d: %s ===%s\n", BOLD, YELLOW, test_num, test->desc, RESET);
	ft_printf("%sInput:%s '%s'\n", YELLOW, RESET, test->input);
	
	// Tokenize
	tokens = lexer(test->input);
	if (!tokens)
	{
		ft_printf("%s✗ FAIL:%s Lexer returned NULL\n", RED, RESET);
		return (0);
	}
	
	// Parse
	ast = parse(tokens);
	
	// Check result
	if (test->expect_error)
	{
		if (ast == NULL)
		{
			ft_printf("%s✓ PASS:%s Parser returned NULL (error expected)\n", GREEN, RESET);
			passed = 1;
		}
		else
		{
			ft_printf("%s✗ FAIL:%s Expected error but got AST:\n", RED, RESET);
			ast_print(ast, 0);
			passed = 0;
		}
	}
	else
	{
		if (ast)
		{
			ft_printf("%s✓ PASS:%s AST created successfully\n", GREEN, RESET);
			ft_printf("\n%sAST Structure:%s\n", CYAN, RESET);
			ast_print(ast, 0);
			passed = 1;
		}
		else
		{
			ft_printf("%s✗ FAIL:%s Expected AST but got NULL\n", RED, RESET);
			passed = 0;
		}
	}
	
	// Cleanup
	ast_free(ast);
	token_lstclear(&tokens);
	
	return (passed);
}

int	main(int ac, char **av, char **envp)
{
	t_shell			shell;
	t_parser_test	tests[] = {
		// ========== SIMPLE COMMANDS ==========
		{
			.input = "ls",
			.expect_error = 0,
			.desc = "Single word command"
		},
		{
			.input = "echo hello world",
			.expect_error = 0,
			.desc = "Command with multiple arguments"
		},
		{
			.input = "ls -la -h",
			.expect_error = 0,
			.desc = "Command with flags"
		},
		{
			.input = "/bin/ls",
			.expect_error = 0,
			.desc = "Command with absolute path"
		},
		{
			.input = "",
			.expect_error = 0,
			.desc = "Empty input (just EOF)"
		},
		
		// ========== SINGLE REDIRECTIONS ==========
		{
			.input = "cat < input.txt",
			.expect_error = 0,
			.desc = "Input redirection"
		},
		{
			.input = "echo hello > output.txt",
			.expect_error = 0,
			.desc = "Output redirection"
		},
		{
			.input = "cat >> append.txt",
			.expect_error = 0,
			.desc = "Append redirection"
		},
		{
			.input = "cat << EOF",
			.expect_error = 0,
			.desc = "Heredoc"
		},
		{
			.input = "< input.txt cat",
			.expect_error = 0,
			.desc = "Redirection before command"
		},
		
		// ========== MULTIPLE REDIRECTIONS ==========
		{
			.input = "cat < in.txt > out.txt",
			.expect_error = 0,
			.desc = "Input and output redirect"
		},
		{
			.input = "cat < in1.txt < in2.txt",
			.expect_error = 0,
			.desc = "Multiple input redirects"
		},
		{
			.input = "echo hello > out1.txt > out2.txt",
			.expect_error = 0,
			.desc = "Multiple output redirects"
		},
		{
			.input = "cat < in > out >> append",
			.expect_error = 0,
			.desc = "Mixed redirections"
		},
		{
			.input = "< in cat file1 file2 > out",
			.expect_error = 0,
			.desc = "Redirects around command"
		},
		
		// ========== SIMPLE PIPES ==========
		{
			.input = "ls | cat",
			.expect_error = 0,
			.desc = "Simple pipe"
		},
		{
			.input = "cat file | grep test",
			.expect_error = 0,
			.desc = "Pipe with arguments"
		},
		{
			.input = "ls | cat | wc",
			.expect_error = 0,
			.desc = "Multiple pipes"
		},
		{
			.input = "cat | grep | wc | sort",
			.expect_error = 0,
			.desc = "Four commands piped"
		},
		
		// ========== PIPES WITH REDIRECTIONS ==========
		{
			.input = "cat < input.txt | grep test",
			.expect_error = 0,
			.desc = "Input redirect with pipe"
		},
		{
			.input = "ls | grep txt > output.txt",
			.expect_error = 0,
			.desc = "Pipe with output redirect"
		},
		{
			.input = "cat < in | grep test > out",
			.expect_error = 0,
			.desc = "Pipe with both redirects"
		},
		{
			.input = "< in cat | grep | wc > out",
			.expect_error = 0,
			.desc = "Multiple pipes with redirects"
		},
		{
			.input = "cat << EOF | grep test | wc -l",
			.expect_error = 0,
			.desc = "Heredoc with pipes"
		},
		
		// ========== COMPLEX COMBINATIONS ==========
		{
			.input = "cat < in1 < in2 | grep test > out1 > out2 | wc",
			.expect_error = 0,
			.desc = "Multiple redirects with multiple pipes"
		},
		{
			.input = "ls -la | grep txt | wc -l > count.txt",
			.expect_error = 0,
			.desc = "Flags, pipes, and output"
		},
		{
			.input = "cat file1 file2 < in | grep pattern > out | sort | uniq",
			.expect_error = 0,
			.desc = "Long pipeline with mixed content"
		},
		
		// ========== VARIABLES ==========
		{
			.input = "echo $HOME",
			.expect_error = 0,
			.desc = "Variable expansion"
		},
		{
			.input = "echo $HOME $USER",
			.expect_error = 0,
			.desc = "Multiple variables"
		},
		{
			.input = "cat $FILE",
			.expect_error = 0,
			.desc = "Variable as argument"
		},
		{
			.input = "cat < $INFILE > $OUTFILE",
			.expect_error = 0,
			.desc = "Variables in redirections"
		},
		{
			.input = "echo $HOME | grep test",
			.expect_error = 0,
			.desc = "Variable with pipe"
		},
		
		// ========== QUOTES ==========
		{
			.input = "echo 'hello world'",
			.expect_error = 0,
			.desc = "Single quoted argument"
		},
		{
			.input = "echo \"hello world\"",
			.expect_error = 0,
			.desc = "Double quoted argument"
		},
		{
			.input = "cat 'file name with spaces.txt'",
			.expect_error = 0,
			.desc = "Quoted filename"
		},
		{
			.input = "echo '$HOME'",
			.expect_error = 0,
			.desc = "Variable in single quotes (literal)"
		},
		{
			.input = "echo \"$HOME\"",
			.expect_error = 0,
			.desc = "Variable in double quotes (expand)"
		},
		
		// ========== SYNTAX ERRORS (Should Fail) ==========
		{
			.input = "| cat",
			.expect_error = 1,
			.desc = "ERROR: Pipe at start"
		},
		{
			.input = "cat |",
			.expect_error = 1,
			.desc = "ERROR: Pipe at end"
		},
		{
			.input = "cat | | grep",
			.expect_error = 1,
			.desc = "ERROR: Double pipe"
		},
		{
			.input = "cat >",
			.expect_error = 1,
			.desc = "ERROR: Redirect without filename"
		},
		{
			.input = "cat <",
			.expect_error = 1,
			.desc = "ERROR: Input redirect without filename"
		},
		{
			.input = "cat > |",
			.expect_error = 1,
			.desc = "ERROR: Redirect followed by pipe"
		},
		{
			.input = "cat > >",
			.expect_error = 1,
			.desc = "ERROR: Redirect followed by redirect"
		},
		
		// ========== EDGE CASES ==========
		{
			.input = "cat||grep",
			.expect_error = 1,
			.desc = "ERROR: No space between pipes"
		},
		{
			.input = "cat<in>out",
			.expect_error = 0,
			.desc = "No spaces around redirects (valid)"
		},
		{
			.input = "<<<",
			.expect_error = 1,
			.desc = "ERROR: Invalid operator"
		},
		{
			.input = ">>>",
			.expect_error = 1,
			.desc = "ERROR: Invalid operator"
		},
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	int i;
	
	(void)ac;
	(void)av;
	
	// Initialize shell
	ft_memset(&shell, 0, sizeof(t_shell));
	init_shell(envp, &shell);
	
	// Print header
	ft_printf("\n%s%s╔═══════════════════════════════════════════════════════════════════╗%s\n", 
			 BOLD, CYAN, RESET);
	ft_printf("%s%s║           MINISHELL PARSER TEST SUITE v1.0                    ║%s\n", 
			 BOLD, CYAN, RESET);
	ft_printf("%s%s╚═══════════════════════════════════════════════════════════════════╝%s\n", 
			 BOLD, CYAN, RESET);
	
	// Run all tests
	for (i = 0; i < num_tests; i++)
	{
		if (run_parser_test(&tests[i], i + 1))
			passed++;
		print_separator();
	}
	
	// Print summary
	ft_printf("\n%s%s════════════════════════ RESULTS ════════════════════════%s\n", 
			 BOLD, CYAN, RESET);
	ft_printf("Total tests: %d\n", num_tests);
	ft_printf("%sPassed: %d%s\n", GREEN, passed, RESET);
	ft_printf("%sFailed: %d%s\n", RED, num_tests - passed, RESET);
	ft_printf("Success rate: %.1f%%\n", (float)passed / num_tests * 100);
	
	if (passed == num_tests)
		ft_printf("\n%s%s🎉 ALL TESTS PASSED! 🎉%s\n\n", 
				 BOLD, GREEN, RESET);
	else
		ft_printf("\n%s%s⚠️  SOME TESTS FAILED ⚠️%s\n\n", 
				 BOLD, RED, RESET);
	
	return (passed == num_tests ? 0 : 1);
}
