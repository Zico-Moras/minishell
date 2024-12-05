#include <unistd.h>
#include <stdio.h>

int	main()
{
	int	pid;
	pid_t	ppid;

	pid = getpid();
	ppid = getppid();
	printf("the pid is: %i\n", pid);
	printf("the ppid is: %i\n", ppid);


}
