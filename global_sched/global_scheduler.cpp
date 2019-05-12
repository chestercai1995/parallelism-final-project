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
#include "shm.h"

using namespace std;

/* =========================================== */
//Globals

uint64_t cpu_stat[4];
pid_t child_pids[4];


/* =========================================== */

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
	int num_programs = 0, pos = 0;
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

	for(int i=0; i<num_programs; i++)
	{
		cpu_stat[i] = 0;
	}

	
	int shmid;
	int *shm_ptr = (int *) get_shared_ptr( (char *) programs[0].c_str(), sizeof(int)*4, SHM_W, &shmid);

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
		printf("Parent\n");
	}



}
