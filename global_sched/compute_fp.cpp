#include <stdio.h>
#include <stdlib.h>

#include "shm.h"
#include "stats.h"

#define computation_loops 1000000

stats_struct *stats;

void * timer_interrupt(int intr)
{
	printf("FP interrupt\n");
	return NULL;
}

void setup_timer()
{
	struct timeval value = {1, 0};
	struct timeval interval = {0, RECORD_STAT_QUANTUM};
	struct itimerval timer = {interval, value};
	
	signal(SIGPROF, (__sighandler_t) timer_interrupt);
	setitimer(ITIMER_PROF, &timer, 0);
	return;
}

inline double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

inline double compute_fp(double in1, double in2)
{
	double const1 = fRand(0, 1);
	double const2 = fRand(-1, 0);
	double const3 = fRand(-1, 1);

	double v1 = const1;
	double v2 = const2;

	
	for(int i=0; i<computation_loops; i++)
	{
		v1 = v1*in1;
		v2 = v2*in2;
		v1 = v1/const3;
		v2 = v2/const3;
	}

	return v1/v2;

}

int main(int argc, char* argv[])
{
	if(argc!=2) {
		printf("Agrument for multiples of runtime needed\n");
		return 1;
	}
	stats = (stats_struct *) calloc(1, sizeof(stats_struct));
	
	setup_timer();
	
	int rt_mul = atoi(argv[1]);
	
	int rep_count = 100*rt_mul;

	int array_size = 10;
	//Initialize small array
	double * small_array;
	small_array =  new double[array_size];

	srand(25);

	for(int k=0; k<rep_count; k++) 
	{

		for(int i=0; i<array_size; i++)
		{
			small_array[i] = fRand(-100, 100);
		} 

		for(int i=2; i<array_size; i++)
		{
			small_array[i] = compute_fp(small_array[i-1], small_array[i-2]);
		} 

	}

	return 0;
}
