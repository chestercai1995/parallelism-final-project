#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

typedef void (*sighandler_t)(int);

foo(int theint)
{
	printf("I just got the SIGALRM signal\n");
}

main()
{
	struct timeval my_value={1,0};
	struct timeval my_interval={1,0};
	struct itimerval my_timer={my_interval,my_value};
	
	setitimer(ITIMER_REAL, &my_timer, 0);
	
	int i;
	signal(SIGALRM, (sighandler_t) foo);

	while(1);
}
