#include "includes/minishell.h"
#include <stdio.h>

// ANSI Colors
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"
#define RESET   "\033[0m"

// Expected argument with quote info
typedef struct s_expected_arg {
	char	*value;
	int		quoted;
} t_expected_arg;

// Expected redirection with quote info
typedef struct s_expected_redir {
	t_node_type	type;
	char		*file;
	int			quoted;
} t_expected_redir;

typedef struct s_parser_test {
	char				*input;
	int					expect_error;
	char				*desc;
	t_expected_arg		*expected_args;		// NULL-terminated
	t_expected_redir	*expected_redirs;	// NULL-terminated
	int					is_pipeline;		// 1 if expecting PIPE node
} t_parser_test;

// Count args in AST node
static int count_args(char **args)
{
	int count = 0;
	
	if (!args)
		return (0);
	while (args[count])
		count++;
	return (count);
}

// Count redirects in list
static int count_redirs(t_redir_node *redirs)
{
	int count = 0;
	t_redir_node *curr = redirs;
	
	while (curr)
	{
		count++;
		curr = curr->next;
	}
	return (count);
}

// Count expected args
static int count_expected_args(t_expected_arg *expected)
{
	int count = 0;
	
	if (!expected)
		return (0);
	while (expected[count].value)
		count++;
	return (count);
}

// Count expected redirs
static int count_expected_redirs(t_expected_redir *expected)
{
	int count = 0;
	
	if (!expected)
		return (0);
	while (expected[count].file)
		count++;
	return (count);
}

// Check if arguments match (including quote info)
static int check_args(t_ast_node *node, t_expected_arg *expected)
{
	int i = 0;
	int exp_count = count_expected_args(expected);
	int actual_count = count_args(node->args);
	
	if (!expected && !node->args)
		return (1);
	
	if (actual_count != exp_count)
	{
		ft_printf("  %s✗ Arg count mismatch:%s expected %d, got %d\n",
				RED, RESET, exp_count, actual_count);
		return (0);
	}
	
	while (expected && expected[i].value)
	{
		if (!node->args[i] || ft_strcmp(node->args[i], expected[i].value) != 0)
		{
			ft_printf("  %s✗ Arg[%d] value mismatch:%s expected '%s', got '%s'\n",
					RED, RESET, i, expected[i].value,
					node->args[i] ? node->args[i] : "NULL");
			return (0);
		}
		
		if (node->args_quoted[i] != expected[i].quoted)
		{
			ft_printf("  %s✗ Arg[%d] quote mismatch:%s '%s' expected quoted=%d, got quoted=%d\n",
					RED, RESET, i, expected[i].value,
					expected[i].quoted, node->args_quoted[i]);
			return (0);
		}
		i++;
	}
	
	return (1);
}

// Check if redirections match (including quote info)
static int check_redirs(t_ast_node *node, t_expected_redir *expected)
{
	int i = 0;
	int exp_count = count_expected_redirs(expected);
	int actual_count = count_redirs(node->redirects);
	t_redir_node *curr = node->redirects;
	
	if (!expected && !node->redirects)
		return (1);
	
	if (actual_count != exp_count)
	{
		ft_printf("  %s✗ Redir count mismatch:%s expected %d, got %d\n",
				RED, RESET, exp_count, actual_count);
		return (0);
	}
	
	while (expected && expected[i].file && curr)
	{
		if (curr->type != expected[i].type)
		{
			ft_printf("  %s✗ Redir[%d] type mismatch:%s expected %d, got %d\n",
					RED, RESET, i, expected[i].type, curr->type);
			return (0);
		}
		
		if (ft_strcmp(curr->file, expected[i].file) != 0)
		{
			ft_printf("  %s✗ Redir[%d] file mismatch:%s expected '%s', got '%s'\n",
					RED, RESET, i, expected[i].file, curr->file);
			return (0);
		}
		
		if (curr->quoted != expected[i].quoted)
		{
			ft_printf("  %s✗ Redir[%d] quote mismatch:%s '%s' expected quoted=%d, got quoted=%d\n",
					RED, RESET, i, expected[i].file,
					expected[i].quoted, curr->quoted);
			return (0);
		}
		
		curr = curr->next;
		i++;
	}
	
	return (1);
}

// Verify command node contents
static int verify_command(t_ast_node *node, t_parser_test *test)
{
	if (node->type != NODE_COMMAND)
	{
		ft_printf("  %s✗ FAIL:%s Expected NODE_COMMAND, got type %d\n",
				RED, RESET, node->type);
		return (0);
	}
	
	if (!check_args(node, test->expected_args))
		return (0);
	
	if (!check_redirs(node, test->expected_redirs))
		return (0);
	
	return (1);
}

static int run_parser_test(t_parser_test *test, int test_num)
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
		if (test->expect_error)
		{
			ft_printf("  %s✓ PASS:%s Lexer returned NULL (error expected)\n", GREEN, RESET);
			return (1);
		}
		else
		{
			ft_printf("  %s✗ FAIL:%s Lexer returned NULL\n", RED, RESET);
			return (0);
		}
	}
	
	// Parse
	ast = parse(tokens);
	
	// Check result
	if (test->expect_error)
	{
		if (ast == NULL)
		{
			ft_printf("  %s✓ PASS:%s Parser returned NULL (error expected)\n", GREEN, RESET);
			passed = 1;
		}
		else
		{
			ft_printf("  %s✗ FAIL:%s Expected error but got AST\n", RED, RESET);
			passed = 0;
		}
	}
	else
	{
		if (!ast)
		{
			ft_printf("  %s✗ FAIL:%s Expected AST but got NULL\n", RED, RESET);
			passed = 0;
		}
		else if (test->is_pipeline)
		{
			if (ast->type == NODE_PIPE)
			{
				ft_printf("  %s✓ PASS:%s Pipeline AST created\n", GREEN, RESET);
				passed = 1;
			}
			else
			{
				ft_printf("  %s✗ FAIL:%s Expected NODE_PIPE, got %d\n", RED, RESET, ast->type);
				passed = 0;
			}
		}
		else
		{
			passed = verify_command(ast, test);
			if (passed)
				ft_printf("  %s✓ PASS:%s Command matches expected\n", GREEN, RESET);
		}
	}
	
	// Cleanup
	if (ast)
		ast_free(ast);
	token_lstclear(&tokens);
	
	return (passed);
}

int main(void)
{
	t_parser_test tests[] = {
		// ========== BASIC COMMANDS ==========
		{
			.input = "echo hello",
			.expect_error = 0,
			.desc = "Simple command with argument",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"hello", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "pwd",
			.expect_error = 0,
			.desc = "Single command no args",
			.expected_args = (t_expected_arg[]){
				{"pwd", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},

		// ========== QUOTES IN ARGUMENTS ==========
		{
			.input = "echo 'single quoted'",
			.expect_error = 0,
			.desc = "Single quoted argument",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"single quoted", 1},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo \"double quoted\"",
			.expect_error = 0,
			.desc = "Double quoted argument",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"double quoted", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo 'hello world'",
			.expect_error = 0,
			.desc = "Single quoted arg with space",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"hello world", 1},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo \"hello world\"",
			.expect_error = 0,
			.desc = "Double quoted arg with space",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"hello world", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo 'nested \"double\" inside single'",
			.expect_error = 0,
			.desc = "Nested double quotes inside single",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"nested \"double\" inside single", 1},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo \"nested 'single' inside double\"",
			.expect_error = 0,
			.desc = "Nested single quotes inside double",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"nested 'single' inside double", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo ''",
			.expect_error = 0,
			.desc = "Empty single quoted arg",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"", 1},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo \"\"",
			.expect_error = 0,
			.desc = "Empty double quoted arg",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo '' \"\" word",
			.expect_error = 0,
			.desc = "Empty quotes with regular word",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"", 1},
				{"", 2},
				{"word", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},

		// ========== OPERATORS INSIDE QUOTES ==========
		{
			.input = "echo '|'",
			.expect_error = 0,
			.desc = "Pipe inside single quotes",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"|", 1},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo \"|\"",
			.expect_error = 0,
			.desc = "Pipe inside double quotes",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"|", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo '< > |'",
			.expect_error = 0,
			.desc = "Multiple operators inside single quotes",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"< > |", 1},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo \"< > |\"",
			.expect_error = 0,
			.desc = "Multiple operators inside double quotes",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"< > |", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},

		// ========== VARIABLES WITH QUOTES ==========
		{
			.input = "echo $VAR",
			.expect_error = 0,
			.desc = "Unquoted variable",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"$VAR", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo '$VAR'",
			.expect_error = 0,
			.desc = "Single quoted variable",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"$VAR", 1},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo \"$VAR\"",
			.expect_error = 0,
			.desc = "Double quoted variable",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"$VAR", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo '$USER' \"$HOME\" $PATH",
			.expect_error = 0,
			.desc = "Same variable in different quote contexts",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"$USER", 1},
				{"$HOME", 2},
				{"$PATH", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		},

		// ========== REDIRECTIONS ==========
		{
			.input = "cat < input.txt",
			.expect_error = 0,
			.desc = "Input redirection",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_IN, "input.txt", 0},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "echo hello > output.txt",
			.expect_error = 0,
			.desc = "Output redirection",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"hello", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_OUT, "output.txt", 0},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "cat >> append.txt",
			.expect_error = 0,
			.desc = "Append redirection",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_APPEND, "append.txt", 0},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "cat << EOF",
			.expect_error = 0,
			.desc = "Heredoc unquoted delimiter",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_HEREDOC, "EOF", 0},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "cat << 'EOF'",
			.expect_error = 0,
			.desc = "Heredoc single quoted delimiter",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_HEREDOC, "EOF", 1},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "cat << \"EOF\"",
			.expect_error = 0,
			.desc = "Heredoc double quoted delimiter",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_HEREDOC, "EOF", 2},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "cat < in.txt > out.txt",
			.expect_error = 0,
			.desc = "Multiple redirections",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_IN, "in.txt", 0},
				{NODE_REDIR_OUT, "out.txt", 0},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},

		// ========== QUOTED REDIRECTIONS ==========
		{
			.input = "cat < 'input file.txt'",
			.expect_error = 0,
			.desc = "Single quoted input redirection with space",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_IN, "input file.txt", 1},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "echo hello > \"output file.txt\"",
			.expect_error = 0,
			.desc = "Double quoted output redirection with space",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"hello", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_OUT, "output file.txt", 2},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "cat < $FILE",
			.expect_error = 0,
			.desc = "Variable as filename (should expand)",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_IN, "$FILE", 0},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "cat < '$FILE'",
			.expect_error = 0,
			.desc = "Single quoted variable as filename (no expand)",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_IN, "$FILE", 1},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "cat < \"$FILE\"",
			.expect_error = 0,
			.desc = "Double quoted variable as filename (expand)",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_IN, "$FILE", 2},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},

		// ========== PIPELINES ==========
		{
			.input = "ls | cat",
			.expect_error = 0,
			.desc = "Simple pipeline",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 1
		},
		{
			.input = "cat file | grep test",
			.expect_error = 0,
			.desc = "Pipeline with arguments",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 1
		},
		{
			.input = "ls | cat | wc",
			.expect_error = 0,
			.desc = "Three-command pipeline",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 1
		},
		{
			.input = "echo \"hello | world\" | cat",
			.expect_error = 0,
			.desc = "Pipeline with quoted arg containing pipe",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 1
		},

		// ========== SYNTAX ERRORS ==========
		{
			.input = "| cat",
			.expect_error = 1,
			.desc = "ERROR: Pipe at start",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "cat |",
			.expect_error = 1,
			.desc = "ERROR: Pipe at end",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "cat | | grep",
			.expect_error = 1,
			.desc = "ERROR: Double pipe",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "cat >",
			.expect_error = 1,
			.desc = "ERROR: Redirect without file",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "cat < > out",
			.expect_error = 1,
			.desc = "ERROR: Redirect without filename",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "cat < < in",
			.expect_error = 1,
			.desc = "ERROR: Double redirect",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},

		// ========== UNCLOSED QUOTES (LEXER ERRORS) ==========
		{
			.input = "echo 'unclosed",
			.expect_error = 1,
			.desc = "ERROR: Unclosed single quote",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "echo \"unclosed",
			.expect_error = 1,
			.desc = "ERROR: Unclosed double quote",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "cat < 'unclosed",
			.expect_error = 1,
			.desc = "ERROR: Unclosed quote in redirection",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},

		// ========== EDGE CASES ==========
		{
			.input = "",
			.expect_error = 0,
			.desc = "Empty input",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "   ",
			.expect_error = 0,
			.desc = "Only whitespace",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0
		},
		{
			.input = "> file",
			.expect_error = 0,
			.desc = "Redirection without command",
			.expected_args = NULL,
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_OUT, "file", 0},
				{0, NULL, 0}
			},
			.is_pipeline = 0
		},
		{
			.input = "echo hello     world",
			.expect_error = 0,
			.desc = "Multiple spaces between args",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"hello", 0},
				{"world", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0
		}
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	int failed = 0;
	
	ft_printf("%s╔═══════════════════════════════════════════════╗%s\n", CYAN, RESET);
	ft_printf("%s║   MINISHELL PARSER TEST SUITE (With Quotes)  ║%s\n", CYAN, RESET);
	ft_printf("%s╚═══════════════════════════════════════════════╝%s\n", CYAN, RESET);
	
	for (int i = 0; i < num_tests; i++)
	{
		if (run_parser_test(&tests[i], i + 1))
			passed++;
		else
			failed++;
	}
	
	ft_printf("\n%s════════════════ RESULTS ═══════════════════%s\n", BOLD, RESET);
	ft_printf("Total tests: %d\n", num_tests);
	ft_printf("Passed: %s%d%s\n", GREEN, passed, RESET);
	ft_printf("Failed: %s%d%s\n", failed > 0 ? RED : GREEN, failed, RESET);
	
	if (failed == 0)
		ft_printf("\n%s🎉 ALL TESTS PASSED! Quote metadata working! 🎉%s\n", GREEN, RESET);
	else
		ft_printf("\n%s⚠️  SOME TESTS FAILED ⚠️%s\n", YELLOW, RESET);
	
	return (0);
}
