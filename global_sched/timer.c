#include <stdio.h>
#include <signal.h>
#include <sys/time.h>



void *foo(int theint)
{
	printf("I just got the SIGPROF signal\n");
}

main()
{
	struct timeval my_value={1,0};
	struct timeval my_interval={0,100000};
	struct itimerval my_timer={my_interval,my_value};
	
	signal(SIGPROF, (__sighandler_t) foo);
	setitimer(ITIMER_PROF, &my_timer, 0);
	
	int i;

	while(1);
}
