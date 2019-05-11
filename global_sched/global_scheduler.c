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
		printf("global_scheduler [program 1] [number of instances] [program 2] number of instances] ... \n");
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
	

	//Launch all programs
	int pid = 0;
	for(int i=0; i<1; i++)
	{
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
		pid = fork();
		if(pid==0) break;
		child_pids[i] = pid;
		printf("%d\n", child_pids[i]);
	}
	
	if(pid == 0) 
	{
		printf("Child\n");
		char *child_argv[] = {argv[1], "1", NULL};
		int ret = execv(argv[1], child_argv);
		if(ret == -1) printf(" %s\n", strerror(errno));
	}
	else
	{
		printf("Parent\n");
	}



}
