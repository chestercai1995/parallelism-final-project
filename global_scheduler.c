#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>


/* =========================================== */

#define _GNU_SOURCE

/* =========================================== */
//Globals

uint64_t cpu_stat[4];
uint64_t child_pids[4];


/* =========================================== */

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("global_scheduler [program 1] [number of instances] [program 2] number of instances] ... \n");
		exit(1);
	}
	
	int num_programs =  argc - 1;
	if(!((argc-1)%2))
	{
		printf("Incorrectly specified programs or number of instances\n");
		exit(1);
	}

	for(int i=0; i<4; i++)
	{
		cpu_stat[i] = 0;
	}

	num_programs = num_programs/2;

	//Launch all programs
	int pid = 0;
	for(int i=0; i<4; i++)
	{
		pid = fork();
		if(pid==0) break;
		child_pids[i] = pid;
		printf("%d\n", child_pids[i]);
	}
	
	if(pid == 0) 
	{
		printf("Child\n");
	}
	else
	{
		printf("Parent\n");
	}



}
