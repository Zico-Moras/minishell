#include "includes/minishell.h"
#include <stdio.h>

// ANSI Colors
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"
#define RESET   "\033[0m"

// Expected token struct
typedef struct s_expected_token {
    t_token_type type;
    char *value;
    int quoted;
} t_expected_token;

// Test case
typedef struct s_test_case {
    char *input;
    t_expected_token *expected;
    int expect_error;
    char *desc;
} t_test_case;

// Count tokens
static int token_count(t_token *tokens)
{
    int count = 0;
    while (tokens)
    {
        count++;
        tokens = tokens->next;
    }
    return (count);
}

// Compare single token
static int compare_token(t_token *actual, t_token_type exp_type, 
                        char *exp_value, int exp_quoted)
{
    if (actual->type != exp_type || actual->quoted != exp_quoted)
        return (0);
    if (exp_value)
    {
        if (!actual->value || ft_strcmp(actual->value, exp_value) != 0)
            return (0);
    }
    else if (actual->value)
        return (0);
    return (1);
}

// Colorized token_print
static void token_print_colored(t_token *token)
{
    const char *type_str[8] = {
        [TOKEN_EOF] = "EOF",
        [TOKEN_WORD] = "WORD",
        [TOKEN_VAR] = "VAR",
        [TOKEN_PIPE] = "PIPE",
        [TOKEN_REDIR_IN] = "REDIR_IN",
        [TOKEN_REDIR_OUT] = "REDIR_OUT",
        [TOKEN_REDIR_APPEND] = "REDIR_APPEND",
        [TOKEN_HEREDOC] = "HEREDOC"
    };
    const char *color = CYAN;
    
    if (!token)
        return;
    
    if (token->type == TOKEN_WORD || token->type == TOKEN_VAR)
        color = GREEN;
    else if (token->type >= TOKEN_PIPE)
        color = RED;
    
    ft_printf("[%s%s%s: '%s' quoted=%d] -> ", color, type_str[token->type], 
              RESET, token->value ? token->value : "NULL", token->quoted);
    
    if (token->next)
        token_print_colored(token->next);
    else
        ft_printf("NULL\n");
}

// Run one lexer test
static int run_lexer_test(t_test_case *tc)
{
    t_token *tokens = lexer(tc->input);
    int passed = 0;
    int exp_count = 0;
    
    if (tc->expected)
    {
        while (tc->expected[exp_count].type != TOKEN_EOF)
            exp_count++;
    }

    ft_printf("%s=== Test: %s ===%s\n", BOLD, tc->desc, RESET);
    ft_printf("%sInput:%s '%s'\n", YELLOW, RESET, 
              tc->input ? tc->input : "(null)");
    
    if (tc->expect_error)
    {
        if (tokens == NULL)
        {
            ft_printf("  %s✓ PASS:%s Returned NULL (error expected)\n", 
                     GREEN, RESET);
            passed = 1;
        }
        else
        {
            ft_printf("  %s✗ FAIL:%s Expected NULL but got tokens\n", 
                     RED, RESET);
            token_print_colored(tokens);
            token_lstclear(&tokens);
        }
        ft_printf("\n");
        return (passed);
    }
    
    if (tokens == NULL)
    {
        ft_printf("  %s✗ FAIL:%s Unexpected NULL\n", RED, RESET);
        ft_printf("\n");
        return (0);
    }
    
    int actual_count = token_count(tokens);
    if (actual_count != exp_count + 1)
    {
        ft_printf("  %s✗ FAIL:%s Token count: got %d, expected %d\n", 
                 RED, RESET, actual_count, exp_count + 1);
        token_print_colored(tokens);
        token_lstclear(&tokens);
        ft_printf("\n");
        return (0);
    }
    
    t_token *curr = tokens;
    passed = 1;
    
    for (int i = 0; i < exp_count; i++)
    {
        if (!compare_token(curr, tc->expected[i].type, 
                          tc->expected[i].value, tc->expected[i].quoted))
        {
            ft_printf("  %s✗ FAIL:%s Mismatch at token %d\n", RED, RESET, i);
            ft_printf("    Expected: type=%d, value='%s', quoted=%d\n",
                      tc->expected[i].type, 
                      tc->expected[i].value ? tc->expected[i].value : "NULL",
                      tc->expected[i].quoted);
            ft_printf("    Got:      type=%d, value='%s', quoted=%d\n",
                      curr->type, 
                      curr->value ? curr->value : "NULL",
                      curr->quoted);
            passed = 0;
            break;
        }
        curr = curr->next;
    }
    
    if (passed && !compare_token(curr, TOKEN_EOF, NULL, 0))
    {
        ft_printf("  %s✗ FAIL:%s Expected TOKEN_EOF, got something else\n", 
                 RED, RESET);
        passed = 0;
    }
    
    if (passed)
    {
        ft_printf("  %s✓ PASS:%s All tokens match\n", GREEN, RESET);
        token_print_colored(tokens);
    }
    
    token_lstclear(&tokens);
    ft_printf("\n");
    return (passed);
}

int main(int ac, char **av, char **envp)
{
    (void)ac;
    (void)av;
    (void)envp;
    
    t_test_case tests[] = {
        // ========== BASIC TESTS ==========
        {
            .input = "ls",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "ls", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Single word"
        },
        {
            .input = "echo hello world",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "hello", 0},
                {TOKEN_WORD, "world", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple words"
        },
        {
            .input = "",
            .expected = (t_expected_token[]){
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Empty input"
        },
        {
            .input = "   \t\n  ",
            .expected = (t_expected_token[]){
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Only whitespace"
        },
        {
            .input = "  ls   -la   ",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "ls", 0},
                {TOKEN_WORD, "-la", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Words with extra whitespace"
        },
        
        // ========== PIPES ==========
        {
            .input = "ls | grep foo",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "ls", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "grep", 0},
                {TOKEN_WORD, "foo", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Simple pipe"
        },
        {
            .input = "cat|grep|wc",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "grep", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "wc", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple pipes no spaces"
        },
        {
            .input = "|||",
            .expected = (t_expected_token[]){
                {TOKEN_PIPE, "|", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Three consecutive pipes"
        },
        {
            .input = "ls | | grep",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "ls", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "grep", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Double pipe with spaces"
        },
        
        // ========== REDIRECTIONS ==========
        {
            .input = "< infile",
            .expected = (t_expected_token[]){
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "infile", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Input redirect"
        },
        {
            .input = "> outfile",
            .expected = (t_expected_token[]){
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_WORD, "outfile", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Output redirect"
        },
        {
            .input = ">> append",
            .expected = (t_expected_token[]){
                {TOKEN_REDIR_APPEND, ">>", 0},
                {TOKEN_WORD, "append", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Append redirect"
        },
        {
            .input = "<< EOF",
            .expected = (t_expected_token[]){
                {TOKEN_HEREDOC, "<<", 0},
                {TOKEN_WORD, "EOF", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Heredoc"
        },
        {
            .input = "cat<infile",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "infile", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Input redirect no spaces"
        },
        {
            .input = "cat>outfile",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_WORD, "outfile", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Output redirect no spaces"
        },
        {
            .input = "cat>>append",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_REDIR_APPEND, ">>", 0},
                {TOKEN_WORD, "append", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Append redirect no spaces"
        },
        {
            .input = "cat<<EOF",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_HEREDOC, "<<", 0},
                {TOKEN_WORD, "EOF", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Heredoc no spaces"
        },
        {
            .input = "<<<",
            .expected = (t_expected_token[]){
                {TOKEN_HEREDOC, "<<", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Triple < tokenizes as heredoc + input"
        },
        {
            .input = ">>>",
            .expected = (t_expected_token[]){
                {TOKEN_REDIR_APPEND, ">>", 0},
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Triple > tokenizes as append + output"
        },
        {
            .input = "cat < in > out",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "in", 0},
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_WORD, "out", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple different redirects"
        },
        {
            .input = "cat < in < in2",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "in", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "in2", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple same redirects"
        },
        
        // ========== SINGLE QUOTES ==========
        {
            .input = "'hello'",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "hello", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Simple single quotes"
        },
        {
            .input = "''",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Empty single quotes"
        },
        {
            .input = "'hello world'",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "hello world", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Single quotes with spaces"
        },
        {
            .input = "'$HOME'",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "$HOME", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "$ in single quotes is literal"
        },
        {
            .input = "'|<>>&'",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "|<>>&", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Operators in single quotes are literal"
        },
        {
            .input = "'hello'world",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "hello", 1},
                {TOKEN_WORD, "world", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Quote followed by unquoted word"
        },
        {
            .input = "hello'world'",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "hello", 0},
                {TOKEN_WORD, "world", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Unquoted word followed by quote"
        },
        {
            .input = "'\\\\'",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "\\\\", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Backslashes in single quotes are literal"
        },
        
        // ========== DOUBLE QUOTES (quoted=2) ==========
        {
            .input = "\"hello\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "hello", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Simple double quotes"
        },
        {
            .input = "\"\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Empty double quotes"
        },
        {
            .input = "\"hello world\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "hello world", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Double quotes with spaces"
        },
        {
            .input = "\"$HOME\"",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$HOME", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Variable in double quotes"
        },
        {
            .input = "\"|<>>&\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "|<>>&", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Operators in double quotes are literal"
        },
        {
            .input = "\"\\\\\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "\\\\", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Backslashes in double quotes are literal"
        },
        {
            .input = "\"hello\"world",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "hello", 2},
                {TOKEN_WORD, "world", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Double quote followed by unquoted"
        },
        
        // ========== MIXED QUOTES ==========
        {
            .input = "''\"\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "", 1},
                {TOKEN_WORD, "", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Empty single then empty double"
        },
        {
            .input = "\"\"''\"\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "", 2},
                {TOKEN_WORD, "", 1},
                {TOKEN_WORD, "", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple consecutive empty quotes"
        },
        {
            .input = "'hello'\"world\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "hello", 1},
                {TOKEN_WORD, "world", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Single quote then double quote"
        },
        {
            .input = "echo 'single' \"double\" unquoted",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "single", 1},
                {TOKEN_WORD, "double", 2},
                {TOKEN_WORD, "unquoted", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Mix of all quote types"
        },
        
        // ========== VARIABLES ==========
        {
            .input = "$HOME",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$HOME", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Simple variable"
        },
        {
            .input = "$",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Lone dollar sign (expander handles)"
        },
        {
            .input = "$?",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$?", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Exit status variable"
        },
        {
            .input = "echo $HOME $USER",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_VAR, "$HOME", 0},
                {TOKEN_VAR, "$USER", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple variables"
        },
        {
            .input = "$HOME$USER",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$HOME$USER", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Concatenated variables"
        },
        {
            .input = "\"$HOME $USER\"",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$HOME $USER", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple vars in double quotes"
        },
        {
            .input = "$HOME'$USER'",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$HOME", 0},
                {TOKEN_WORD, "$USER", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Var then single quoted var (literal)"
        },
        {
            .input = "$HOME\"$USER\"",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$HOME", 0},
                {TOKEN_VAR, "$USER", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Var then double quoted var"
        },
        
        // ========== SPECIAL CHARACTERS (literal, not interpreted) ==========
        {
            .input = "echo ; ls",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, ";", 0},
                {TOKEN_WORD, "ls", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Semicolon is a word (not special)"
        },
        {
            .input = "echo hello\\world",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "hello\\world", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Backslash is literal (not escape)"
        },
        {
            .input = "echo \\n\\t\\r",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "\\n\\t\\r", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Escape sequences are literal"
        },
        {
            .input = "echo \\$HOME",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_VAR, "\\$HOME", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Backslash before $ is literal (contains $, so VAR)"
        },
        {
            .input = "echo a\\",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "a\\", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Trailing backslash is literal"
        },
        
        // ========== COMPLEX COMBINATIONS ==========
        {
            .input = "cat < in | grep \"pattern\" > out",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "in", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "grep", 0},
                {TOKEN_WORD, "pattern", 2},
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_WORD, "out", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Pipes and redirects with quotes"
        },
        {
            .input = "<< EOF cat | grep pat >> out",
            .expected = (t_expected_token[]){
                {TOKEN_HEREDOC, "<<", 0},
                {TOKEN_WORD, "EOF", 0},
                {TOKEN_WORD, "cat", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "grep", 0},
                {TOKEN_WORD, "pat", 0},
                {TOKEN_REDIR_APPEND, ">>", 0},
                {TOKEN_WORD, "out", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Heredoc with pipe and append"
        },
        {
            .input = "cat<in>out|grep>>app",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "in", 0},
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_WORD, "out", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "grep", 0},
                {TOKEN_REDIR_APPEND, ">>", 0},
                {TOKEN_WORD, "app", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "All operators no spaces"
        },
        {
            .input = "echo \"test $USER\" | cat < 'file name'",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_VAR, "test $USER", 2},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "cat", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "file name", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Quoted strings with spaces and vars (contains $, so VAR)"
        },
        {
            .input = "echo \"a;b|c>d<e\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "a;b|c>d<e", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Special chars in quotes are literal"
        },
        {
            .input = "cat|\"grep\"|'wc'<\"file\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "grep", 2},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "wc", 1},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "file", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Quoted commands and filenames"
        },
        
        // ========== BUILTINS ==========
        {
            .input = "echo -n hello",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "-n", 0},
                {TOKEN_WORD, "hello", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "echo with -n option"
        },
        {
            .input = "cd /home/user",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cd", 0},
                {TOKEN_WORD, "/home/user", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "cd with absolute path"
        },
        {
            .input = "cd ..",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cd", 0},
                {TOKEN_WORD, "..", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "cd with relative path"
        },
        {
            .input = "pwd",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "pwd", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "pwd (no options)"
        },
        {
            .input = "export VAR=value",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "export", 0},
                {TOKEN_WORD, "VAR=value", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "export with assignment"
        },
        {
            .input = "export VAR=\"hello world\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "export", 0},
                {TOKEN_WORD, "VAR=", 0},
                {TOKEN_WORD, "hello world", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "export with quoted value"
        },
        {
            .input = "unset VAR",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "unset", 0},
                {TOKEN_WORD, "VAR", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "unset variable"
        },
        {
            .input = "env",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "env", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "env (no options)"
        },
        {
            .input = "exit",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "exit", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "exit (no options)"
        },
        {
            .input = "exit 42",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "exit", 0},
                {TOKEN_WORD, "42", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "exit with code"
        },
        
        // ========== UNCLOSED QUOTES (ERRORS) ==========
        {
            .input = "'",
            .expect_error = 1,
            .desc = "Single lone single quote"
        },
        {
            .input = "\"",
            .expect_error = 1,
            .desc = "Single lone double quote"
        },
        {
            .input = "'hello",
            .expect_error = 1,
            .desc = "Unclosed single quote"
        },
        {
            .input = "\"hello",
            .expect_error = 1,
            .desc = "Unclosed double quote"
        },
        {
            .input = "echo 'hello world",
            .expect_error = 1,
            .desc = "Unclosed single quote at end"
        },
        {
            .input = "echo \"hello world",
            .expect_error = 1,
            .desc = "Unclosed double quote at end"
        },
        {
            .input = "'hello' 'world",
            .expect_error = 1,
            .desc = "Valid quote then unclosed"
        },
        {
            .input = "\"hello\" \"world",
            .expect_error = 1,
            .desc = "Valid double quote then unclosed"
        },
        {
            .input = "cat < 'file",
            .expect_error = 1,
            .desc = "Unclosed quote after redirect"
        },
        {
            .input = "echo | 'grep",
            .expect_error = 1,
            .desc = "Unclosed quote after pipe"
        },
        {
            .input = "'hello | grep",
            .expect_error = 1,
            .desc = "Unclosed quote with pipe inside"
        },
        {
            .input = "echo 'hello > out",
            .expect_error = 1,
            .desc = "Unclosed with redirect inside"
        },
        {
            .input = "ls 'test",
            .expect_error = 1,
            .desc = "Simple unclosed single after word"
        },
        {
            .input = "ls \"test",
            .expect_error = 1,
            .desc = "Simple unclosed double after word"
        },
        {
            .input = "'hello' | \"world",
            .expect_error = 1,
            .desc = "Valid single, pipe, unclosed double"
        },
        {
            .input = "echo '$HOME",
            .expect_error = 1,
            .desc = "Unclosed single with variable inside"
        },
        {
            .input = "echo \"$HOME",
            .expect_error = 1,
            .desc = "Unclosed double with variable inside"
        },
        {
            .input = "'a'b'c",
            .expect_error = 1,
            .desc = "Multiple quotes, last unclosed"
        },
        {
            .input = "\"a\"b\"c",
            .expect_error = 1,
            .desc = "Multiple double quotes, last unclosed"
        },
        {
            .input = "echo '",
            .expect_error = 1,
            .desc = "Word followed by single quote only"
        },
        {
            .input = "echo \"",
            .expect_error = 1,
            .desc = "Word followed by double quote only"
        },
        {
            .input = "' ' '",
            .expect_error = 1,
            .desc = "Valid empty quote then unclosed"
        },
        {
            .input = "\" \" \"",
            .expect_error = 1,
            .desc = "Valid empty double quote then unclosed"
        },
        {
            .input = "echo 'don't'",
            .expect_error = 1,
            .desc = "Quote inside same quote type causes error"
        },
        {
            .input = "echo \"can't\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "can't", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Single quote inside double is literal"
        },
        
        // ========== EDGE CASES & STRESS TESTS ==========
        {
            .input = "echo \"\"\"\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "", 2},
                {TOKEN_WORD, "", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Four consecutive double quotes (two empty)"
        },
        {
            .input = "echo ''''",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "", 1},
                {TOKEN_WORD, "", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Four consecutive single quotes (two empty)"
        },
        {
            .input = "$$",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$$", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple lone dollar signs (treated as one token)"
        },
        {
            .input = "$ $ $",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$", 0},
                {TOKEN_VAR, "$", 0},
                {TOKEN_VAR, "$", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple lone dollar signs with spaces"
        },
        {
            .input = "echo $",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_VAR, "$", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Dollar sign at end (expander handles)"
        },
        {
            .input = "echo $ text",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_VAR, "$", 0},
                {TOKEN_WORD, "text", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Dollar sign followed by space (expander handles)"
        },
        {
            .input = "$HOME$",
            .expected = (t_expected_token[]){
                {TOKEN_VAR, "$HOME$", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Variable with trailing $ (expander handles)"
        },
        {
            .input = "echo   ''   \"\"   ",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "", 1},
                {TOKEN_WORD, "", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Empty quotes with lots of whitespace"
        },
        {
            .input = "><",
            .expected = (t_expected_token[]){
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Output then input redirect no spaces"
        },
        {
            .input = "<>",
            .expected = (t_expected_token[]){
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Input then output redirect no spaces"
        },
        {
            .input = "|",
            .expected = (t_expected_token[]){
                {TOKEN_PIPE, "|", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Single pipe"
        },
        {
            .input = "| |",
            .expected = (t_expected_token[]){
                {TOKEN_PIPE, "|", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Two pipes with space"
        },
        {
            .input = "< >",
            .expected = (t_expected_token[]){
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Input and output redirect with space"
        },
        {
            .input = "echo '\t\n'",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "\t\n", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Literal tab and newline in quotes"
        },
        {
            .input = "a''b",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "a", 0},
                {TOKEN_WORD, "", 1},
                {TOKEN_WORD, "b", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Empty quotes between characters"
        },
        {
            .input = "''a''",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "", 1},
                {TOKEN_WORD, "a", 0},
                {TOKEN_WORD, "", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Character surrounded by empty quotes"
        },
        {
            .input = "echo '  spaces  '",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "  spaces  ", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Preserve spaces in quotes"
        },
        {
            .input = "echo \"  spaces  \"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "  spaces  ", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Preserve spaces in double quotes"
        },
        {
            .input = "ls -la | grep test | wc -l | cat",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "ls", 0},
                {TOKEN_WORD, "-la", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "grep", 0},
                {TOKEN_WORD, "test", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "wc", 0},
                {TOKEN_WORD, "-l", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "cat", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Long pipeline"
        },
        {
            .input = "cat < in1 < in2 > out1 > out2 >> app",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "cat", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "in1", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "in2", 0},
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_WORD, "out1", 0},
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_WORD, "out2", 0},
                {TOKEN_REDIR_APPEND, ">>", 0},
                {TOKEN_WORD, "app", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple redirections of all types"
        },
        {
            .input = "echo '$HOME' \"$USER\" $PATH '$?' \"$?\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "$HOME", 1},
                {TOKEN_VAR, "$USER", 2},
                {TOKEN_VAR, "$PATH", 0},
                {TOKEN_WORD, "$?", 1},
                {TOKEN_VAR, "$?", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Mix of literal and expanded variables"
        },
        {
            .input = "echo \"'single in double'\"",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "'single in double'", 2},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Single quotes inside double (literal)"
        },
        {
            .input = "echo '\"double in single\"'",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "\"double in single\"", 1},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Double quotes inside single (literal)"
        },
        {
            .input = "echo ; ; ;",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, ";", 0},
                {TOKEN_WORD, ";", 0},
                {TOKEN_WORD, ";", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Multiple semicolons (treated as words)"
        },
        {
            .input = "echo \\| \\> \\< \\&",
            .expected = (t_expected_token[]){
                {TOKEN_WORD, "echo", 0},
                {TOKEN_WORD, "\\", 0},
                {TOKEN_PIPE, "|", 0},
                {TOKEN_WORD, "\\", 0},
                {TOKEN_REDIR_OUT, ">", 0},
                {TOKEN_WORD, "\\", 0},
                {TOKEN_REDIR_IN, "<", 0},
                {TOKEN_WORD, "\\&", 0},
                {TOKEN_EOF, NULL, 0}
            },
            .expect_error = 0,
            .desc = "Backslashes with operators (lexer separates)"
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    int failed = 0;
    
    ft_printf("%s╔═══════════════════════════════════════╗%s\n", CYAN, RESET);
    ft_printf("%s║   MINISHELL LEXER TEST SUITE v3.0    ║%s\n", CYAN, RESET);
    ft_printf("%s╚═══════════════════════════════════════╝%s\n", CYAN, RESET);
    
    for (int i = 0; i < num_tests; i++)
    {
        if (run_lexer_test(&tests[i]))
            passed++;
        else
            failed++;
    }
    
    ft_printf("%s════════════════ RESULTS ═══════════════=%s\n", BOLD, RESET);
    ft_printf("Total tests: %d\n", num_tests);
    ft_printf("Passed: %s%d%s\n", GREEN, passed, RESET);
    ft_printf("Failed: %s%d%s\n", failed > 0 ? RED : GREEN, failed, RESET);
    
    if (failed == 0)
        ft_printf("\n%s🎉 ALL TESTS PASSED! 🎉%s\n", GREEN, RESET);
    else
        ft_printf("\n%s⚠️  SOME TESTS FAILED ⚠️%s\n", YELLOW, RESET);
    
    return (0);
}
