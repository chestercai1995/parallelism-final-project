#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/wait.h>

#include "shm.h"
#include "stats.h"

using namespace std;

/* =========================================== */
//Globals

uint64_t cpu_stat[4];
pid_t child_pids[4];
int num_programs;
uint64_t **shm_ptrs;

/* =========================================== */

void *global_scheduler(int intr)
{
	for(int i=0; i<num_programs; i++)
	{
		printf("Reading from Proc %d :", i);
		printf("%lu, %lu, %lu, %lu, %lu\n", shm_ptrs[i][0], shm_ptrs[i][1], shm_ptrs[i][2], shm_ptrs[i][3], shm_ptrs[i][4]);
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("global_scheduler [file with list of programs] \n");
		exit(1);
	}

	ifstream in(argv[1]);
	
	if(!in)
	{	
		printf("Input file not found\n");
		exit(1);
	}

	string str;
	num_programs = 0;
	int pos = 0;
	vector<string> programs;
	vector<string> args1;
	vector<string> initial_affinity;

	
	while(getline(in, str))
	{
		string delimiter = " "; 
		pos = str.find(delimiter);
		string token = str.substr(0, pos);
		programs.push_back(token);
		str.erase(0, pos + delimiter.length());
		
		pos = str.find(" ");
		token = str.substr(0, pos);
		args1.push_back(token);
		str.erase(0, pos + delimiter.length());
		
		pos = str.find(" ");
		token = str.substr(0, pos);
		initial_affinity.push_back(token);
		str.erase(0, pos + delimiter.length());
	}
	
	num_programs = programs.size();

	int *shmids = new int[num_programs];
	shm_ptrs = new uint64_t *[num_programs];

	for(int i=0; i<num_programs; i++)
	{
		shm_ptrs[i] = (uint64_t *) get_shared_ptr( (char *) programs[i].c_str(), 64, SHM_RDONLY, &shmids[i]);
	}

	struct timeval value = {1, 0};
	struct timeval interval = {0, GLOBAL_SCHED_QUANTUM};
	struct itimerval timer = {interval, value};
	
	signal(SIGPROF, (__sighandler_t) global_scheduler);
	setitimer(ITIMER_PROF, &timer, 0);

	



	//Launch all programs
	int pid = 0;
	int i = 0;
	cpu_set_t set;
	CPU_ZERO (&set);
	
	for(i=0; i<num_programs; i++)
	{
		pid = fork();
		if(pid==0) break;
		child_pids[i] = pid;
		printf("%d\n", child_pids[i]);
	}
	
	if(pid == 0) 
	{
		CPU_SET(stoi(initial_affinity[i]), &set);
		if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
		{
			printf("Uable to set affinity to %d\n", stoi(initial_affinity[i]));
			exit(2);
		}

		printf("Child %d\n", i);
		char *child_argv[] = {(char *) programs[i].c_str(), (char *) args1[i].c_str(), NULL};
		printf("Child_argv %s %s\n", programs[i].c_str(), args1[i].c_str());
		int ret = execv(child_argv[0], child_argv);
		if(ret == -1) printf(" %s\n", strerror(errno));
	}
	else
	{
		for(int i=0; i<num_programs; i++)
		{
			int status;
			pid_t done = wait(&status);
		}
		for(int i=0; i<num_programs; i++)
		{
			detach_shared_mem(shm_ptrs[i]);
			destroy_shared_mem(&shmids[i]);
		}
		printf("Parent\n");
		
	}



}
