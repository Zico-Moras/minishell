#include "includes/minishell.h"

void	sigint_handler(int sig)
{
	(void)sig;
	if (-1 == waitpid(-1, NULL, WNOHANG))
	{
		write(1, "\n", 1);
		rl_on_new_line();
		rl_replace_line("", 0);
		rl_redisplay();
	}
}

// funcao para dar handle ao sinais
// quando recebe ctrl-c (SIGINT) chama a funcao sigint_handler
void	setup_signals(void)
{
	struct sigaction	sa;

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = &sigint_handler;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}
