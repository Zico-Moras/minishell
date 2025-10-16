#include "includes/minishell.h"
#include <stdio.h>

// ANSI Colors
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"
#define RESET   "\033[0m"

// Expected argument after expansion
typedef struct s_expected_arg {
	char	*value;  // Expected expanded value
	int		quoted;  // Original quoted status (for reference)
} t_expected_arg;

// Expected redirection after expansion
typedef struct s_expected_redir {
	t_node_type	type;
	char		*file;   // Expected expanded file/delimiter
	int			quoted;  // Original quoted status
} t_expected_redir;

typedef struct s_expander_test {
	char				*input;
	int					expect_error;  // 1 if lexer/parser should fail
	char				*desc;
	t_expected_arg		*expected_args;		// NULL-terminated
	t_expected_redir	*expected_redirs;	// NULL-terminated
	int					is_pipeline;		// 1 if expecting PIPE node (basic check only)
	int					exit_status;		// Mock exit status for $?
	char				**mock_envp;		// Mock environment for test
} t_expander_test;

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

// Check if arguments match after expansion (value and quoted preserved)
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

// Check if redirections match after expansion
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

// Verify command node after expansion
static int verify_command(t_ast_node *node, t_expander_test *test)
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

// Mock shell init for tests
static t_shell *create_mock_shell(int exit_status, char **mock_envp)
{
	t_shell *shell = malloc(sizeof(t_shell));
	if (!shell)
		return (NULL);
	ft_memset(shell, 0, sizeof(t_shell));
	shell->exit_status = exit_status;
	shell->envp = args_dup(mock_envp);  // Duplicate the mock envp
	return (shell);
}

// Free mock shell
static void free_mock_shell(t_shell *shell)
{
	if (shell)
	{
		free_array(shell->envp);
		free(shell);
	}
}

static int run_expander_test(t_expander_test *test, int test_num)
{
	t_token		*tokens;
	t_ast_node	*ast;
	t_shell		*mock_shell;
	int			passed;

	ft_printf("\n%s%s=== Test %d: %s ===%s\n", BOLD, YELLOW, test_num, test->desc, RESET);
	ft_printf("%sInput:%s '%s'\n", YELLOW, RESET, test->input);
	
	// Create mock shell
	mock_shell = create_mock_shell(test->exit_status, test->mock_envp);
	if (!mock_shell)
	{
		ft_printf("  %s✗ FAIL:%s Mock shell creation failed\n", RED, RESET);
		return (0);
	}
	
	// Tokenize
	tokens = lexer(test->input);
	if (!tokens)
	{
		if (test->expect_error)
		{
			ft_printf("  %s✓ PASS:%s Lexer returned NULL (error expected)\n", GREEN, RESET);
			free_mock_shell(mock_shell);
			return (1);
		}
		else
		{
			ft_printf("  %s✗ FAIL:%s Lexer returned NULL\n", RED, RESET);
			free_mock_shell(mock_shell);
			return (0);
		}
	}
	
	// Parse
	ast = parse(tokens);
	
	// Expand and check
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
		else
		{
			// Apply expander
			if (!expand_ast(ast, mock_shell))
			{
				ft_printf("  %s✗ FAIL:%s Expansion failed\n", RED, RESET);
				passed = 0;
			}
			else if (test->is_pipeline)
			{
				if (ast->type == NODE_PIPE)
				{
					ft_printf("  %s✓ PASS:%s Pipeline AST expanded (basic check)\n", GREEN, RESET);
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
					ft_printf("  %s✓ PASS:%s Expanded command matches expected\n", GREEN, RESET);
			}
		}
	}
	
	// Cleanup
	if (ast)
		ast_free(ast);
	token_lstclear(&tokens);
	free_mock_shell(mock_shell);
	
	return (passed);
}

int main(void)
{
	// Mock environment used across tests (can be overridden per test)
	char *default_envp[] = {
		"USER=testuser",
		"HOME=/home/test",
		"PATH=/usr/bin",
		"UNDEFINED=",  // For testing empty expansion
		NULL
	};

	t_expander_test tests[] = {
		// ========== BASIC EXPANSION ==========
		{
			.input = "echo $USER",
			.expect_error = 0,
			.desc = "Unquoted variable expansion",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"testuser", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo '$USER'",
			.expect_error = 0,
			.desc = "Single quoted - no expansion",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"$USER", 1},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo \"$USER\"",
			.expect_error = 0,
			.desc = "Double quoted - expansion",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"testuser", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},

		// ========== SPECIAL VARIABLE $? ==========
		{
			.input = "echo $?",
			.expect_error = 0,
			.desc = "Exit status unquoted",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"42", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 42,
			.mock_envp = default_envp
		},
		{
			.input = "echo '$?'",
			.expect_error = 0,
			.desc = "Exit status single quoted - no expand",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"$?", 1},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 42,
			.mock_envp = default_envp
		},
		{
			.input = "echo \"$?\"",
			.expect_error = 0,
			.desc = "Exit status double quoted - expand",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"42", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 42,
			.mock_envp = default_envp
		},

		// ========== UNDEFINED VARIABLES ==========
		{
			.input = "echo $UNDEFINED",
			.expect_error = 0,
			.desc = "Undefined variable - empty string",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo \"$UNDEFINED\"",
			.expect_error = 0,
			.desc = "Undefined in double quotes - empty",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},

		// ========== CONSECUTIVE AND MIXED ==========
		{
			.input = "echo $USER$HOME",
			.expect_error = 0,
			.desc = "Consecutive variables unquoted",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"testuser/home/test", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo \"$USER$HOME\"",
			.expect_error = 0,
			.desc = "Consecutive in double quotes",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"testuser/home/test", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo $USER $HOME",
			.expect_error = 0,
			.desc = "Multiple variables as separate args",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"testuser", 0},
				{"/home/test", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo hello$USER world",
			.expect_error = 0,
			.desc = "Variable embedded in unquoted string",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"hellotestuser", 0},
				{"world", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo \"hello$USER world\"",
			.expect_error = 0,
			.desc = "Embedded in double quotes",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"hellotestuser world", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},

		// ========== INVALID VARIABLE NAMES ==========
		{
			.input = "echo $123",
			.expect_error = 0,
			.desc = "Invalid variable name (starts with digit)",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"$123", 0},  // Should treat as literal $ followed by 123
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo $",
			.expect_error = 0,
			.desc = "Lone $",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"$", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo $$",
			.expect_error = 0,
			.desc = "Double $$",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"$$", 0},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},

		// ========== EXPANSION IN REDIRECTIONS ==========
		{
			.input = "cat < $HOME/file.txt",
			.expect_error = 0,
			.desc = "Variable in redirection filename unquoted",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_IN, "/home/test/file.txt", 0},
				{0, NULL, 0}
			},
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "cat < '$HOME/file.txt'",
			.expect_error = 0,
			.desc = "Single quoted redirection - no expand",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_IN, "$HOME/file.txt", 1},
				{0, NULL, 0}
			},
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "cat < \"$HOME/file.txt\"",
			.expect_error = 0,
			.desc = "Double quoted redirection - expand",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_REDIR_IN, "/home/test/file.txt", 2},
				{0, NULL, 0}
			},
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "cat << $USER",
			.expect_error = 0,
			.desc = "Heredoc delimiter with variable unquoted - expand",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_HEREDOC, "testuser", 0},
				{0, NULL, 0}
			},
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "cat << '$USER'",
			.expect_error = 0,
			.desc = "Heredoc single quoted delimiter - no expand",
			.expected_args = (t_expected_arg[]){
				{"cat", 0},
				{NULL, 0}
			},
			.expected_redirs = (t_expected_redir[]){
				{NODE_HEREDOC, "$USER", 1},
				{0, NULL, 0}
			},
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},

		// ========== PIPELINES WITH EXPANSION ==========
		{
			.input = "echo $USER | grep test",
			.expect_error = 0,
			.desc = "Pipeline with expansion in first command",
			.expected_args = NULL,  // Basic check only for pipeline
			.expected_redirs = NULL,
			.is_pipeline = 1,
			.exit_status = 0,
			.mock_envp = default_envp
		},

		// ========== EDGE CASES ==========
		{
			.input = "echo \"$USER $HOME\"",
			.expect_error = 0,
			.desc = "Multiple variables in one quoted arg",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"testuser /home/test", 2},
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo ''$USER''",
			.expect_error = 0,
			.desc = "Variable between empty quotes",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"testuser", 0},  // Quotes are empty, so just expanded var
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo $UNDEFINED$value",  // $value not defined
			.expect_error = 0,
			.desc = "Undefined followed by text",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"value", 0},  // Undefined expands to empty
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},
		{
			.input = "echo \"\\$USER\"",
			.expect_error = 0,
			.desc = "Escaped $ in double quotes (no expansion if escape handled, but your code doesn't handle escapes yet)",
			.expected_args = (t_expected_arg[]){
				{"echo", 0},
				{"\\testuser", 2},  // Since no escape handling, $ expands
				{NULL, 0}
			},
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		},

		// ========== SYNTAX ERRORS (FOR COMPLETENESS) ==========
		{
			.input = "echo $USER |",
			.expect_error = 1,
			.desc = "ERROR: Invalid syntax (should fail before expansion)",
			.expected_args = NULL,
			.expected_redirs = NULL,
			.is_pipeline = 0,
			.exit_status = 0,
			.mock_envp = default_envp
		}
	};
	
	int num_tests = sizeof(tests) / sizeof(tests[0]);
	int passed = 0;
	int failed = 0;
	
	ft_printf("%s╔═══════════════════════════════════════════════╗%s\n", CYAN, RESET);
	ft_printf("%s║   MINISHELL EXPANDER TEST SUITE               ║%s\n", CYAN, RESET);
	ft_printf("%s╚═══════════════════════════════════════════════╝%s\n", CYAN, RESET);
	
	for (int i = 0; i < num_tests; i++)
	{
		if (run_expander_test(&tests[i], i + 1))
			passed++;
		else
			failed++;
	}
	
	ft_printf("\n%s════════════════ RESULTS ═══════════════════%s\n", BOLD, RESET);
	ft_printf("Total tests: %d\n", num_tests);
	ft_printf("Passed: %s%d%s\n", GREEN, passed, RESET);
	ft_printf("Failed: %s%d%s\n", failed > 0 ? RED : GREEN, failed, RESET);
	
	if (failed == 0)
		ft_printf("\n%s🎉 ALL TESTS PASSED! Expander working! 🎉%s\n", GREEN, RESET);
	else
		ft_printf("\n%s⚠️  SOME TESTS FAILED ⚠️%s\n", YELLOW, RESET);
	
	return (0);
}
