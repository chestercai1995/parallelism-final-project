#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "shm.h"

/* =========================================== */

#define _GNU_SOURCE

/* =========================================== */
//Globals

uint64_t cpu_stat[4];
pid_t child_pids[4];


/* =========================================== */

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("global_scheduler [program 1] [iterations] [program 2] [iterations] ... \n");
		exit(1);
	}
	
	int num_programs =  argc - 1;
	if((argc-1)%2)
	{
		printf("Incorrectly specified programs or number of instances\n");
		exit(1);
	}

	for(int i=0; i<4; i++)
	{
		cpu_stat[i] = 0;
	}

	num_programs = num_programs/2;
	
	int shmid;
	int *shm_ptr = (int *) get_shared_ptr(argv[1], sizeof(int)*4, SHM_W, &shmid);

	printf("Writing to SHM\n");
	shm_ptr[0] = 10;
	shm_ptr[1] = 20;
	shm_ptr[2] = 30;
	shm_ptr[3] = 40;

	printf("Done writing\n");
	fflush(stdout);
	detach_shared_mem(shm_ptr);	

	//Launch all programs
	int pid = 0;
	int i = 0;
	for(i=0; i<num_programs; i++)
	{
		pid = fork();
		if(pid==0) break;
		child_pids[i] = pid;
		printf("%d\n", child_pids[i]);
	}
	
	if(pid == 0) 
	{
		printf("Child %d\n", i);
		char *child_argv[] = {argv[2*i+1], argv[2*i+2], NULL};
		printf("Child_argv %s %s\n", child_argv[0], child_argv[1]);
		int ret = execv(child_argv[0], child_argv);
		if(ret == -1) printf(" %s\n", strerror(errno));
	}
	else
	{
		printf("Parent\n");
	}



}
