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

int num_programs;

stats_struct *stats_ptrs;

core_write_struct **core_mapping;


int shmid;
int *shmids;

//Global stats
stats_struct *core_stats;

int my_tile = 0;

/* =========================================== */


void *global_scheduler(int intr)
{
	int i = 2*my_tile;

    if(my_tile==1 || my_tile==0){
    //if(*(core_mapping[i])!=-1)
    //{
		printf("Reading from Proc %d :", i);
		stats_struct * ptr = &stats_ptrs[i];
		printf("%ld, %ld, %ld, %ld, %ld\n", ptr->l2_cache_misses, ptr->l2_cache_accesses, ptr->num_instructions, ptr->num_cycles, ptr->num_ref_cycles);
	//}
    //if(*(core_mapping[i])!=-1)
    //{
	    i = 2*my_tile+1;
		
        printf("Reading from Proc %d :", i);
		ptr = &stats_ptrs[i];
		printf("%ld, %ld, %ld, %ld, %ld\n", ptr->l2_cache_misses, ptr->l2_cache_accesses, ptr->num_instructions, ptr->num_cycles, ptr->num_ref_cycles);

    //}
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
	vector<int> initial_affinity;

	
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
		initial_affinity.push_back(std::stoi(token));
		str.erase(0, pos + delimiter.length());
	}
	
	num_programs = programs.size();

	core_stats = new stats_struct[68];
	shmids = new int[68];
	core_mapping = new core_write_struct *[68];
	

	stats_ptrs = (stats_struct *) get_shared_ptr( "stats_pt", sizeof(stats_struct)*68, SHM_W, &shmid);

    stats_struct tryi = stats_ptrs[3];

	for(int i=0; i<68; i++)
	{
        bool created = false;
        for(int k=0; k<num_programs; k++) 
        {
            if(initial_affinity[k] == i)
            {
                core_mapping[k] = (core_write_struct *) get_shared_ptr( (char *) programs[k].c_str(), sizeof(core_write_struct), SHM_W, &shmids[k]);
                core_mapping[k]->core_write_id = i;
                //Add pid to stats_struct
                created= true;
                break;
            }
        }
    
	}




	//Launch all programs
	int pid = 0;
	int i = 0;
	cpu_set_t set;
	CPU_ZERO (&set);
	
	for(i=0; i<num_programs; i++)
	{
		pid = fork();
		if(pid==0) break;
        (&stats_ptrs[initial_affinity[i]])->pid = pid;
        (&stats_ptrs[initial_affinity[i]])->mapping_index = i;
	}
	
	if(pid == 0) 
	{
		CPU_SET((initial_affinity[i]), &set);
		if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
		{
			printf("Uable to set affinity to %d\n", initial_affinity[i]);
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

		vector<pid_t> child_scheds;
		//Fork child schedulers
		int sched_pid = 0;
		for(i=0; i<(34-1); i++)
		{
			sched_pid = fork();
			if(sched_pid==0) break;
			child_scheds.push_back(sched_pid);
		}
	
		if(sched_pid == 0)
		{
            
            //Recreate all ptrs for children
            stats_ptrs = (stats_struct *) get_shared_ptr( "stats_pt", sizeof(stats_struct)*68, SHM_W, &shmid);

            for(int i=0; i<68; i++)
            {
                bool created = false;
                for(int k=0; k<num_programs; k++) 
                {
                    if(initial_affinity[k] == i)
                    {
                        core_mapping[k] = (core_write_struct *) get_shared_ptr( (char *) programs[k].c_str(), sizeof(core_write_struct), SHM_W, &shmids[k]);
                        core_mapping[k]->core_write_id = i;
                        created= true;
                        break;
                    }
                }
            
            }

			
            my_tile = i+1;
			printf("Child sched %d\n", my_tile);
			struct timeval value = {1, 0};
			struct timeval interval = {0, GLOBAL_SCHED_QUANTUM};
			struct itimerval timer = {interval, value};
			
			signal(SIGALRM, (__sighandler_t) global_scheduler);
			setitimer(ITIMER_REAL, &timer, 0);


			while(1) pause();
		}
		else
		{
			
			printf("Parent sched %d\n", my_tile);
			struct timeval value = {1, 0};
			struct timeval interval = {0, GLOBAL_SCHED_QUANTUM};
			struct itimerval timer = {interval, value};
			
			signal(SIGALRM, (__sighandler_t) global_scheduler);
			setitimer(ITIMER_REAL, &timer, 0);
			
				
			
			for(int i=0; i<num_programs; i++)
			{
				int status;
				pid_t done = waitpid(-1, &status, 0);
				if(done!=0 || done!=-1)
				{
					printf("Done %d\n", done);
				}
			}
			
		 	while (!child_scheds.empty())
			{
                printf("Killing %d\n", child_scheds.back());
                printf("returned %d\n", kill(child_scheds.back(), SIGKILL));
				child_scheds.pop_back();
			}
				
            detach_shared_mem(stats_ptrs);
            destroy_shared_mem(&shmid);

			for(int i=0; i<num_programs; i++)
			{
				detach_shared_mem(core_mapping[i]);
				destroy_shared_mem(&shmids[i]);
			}
			printf("Parent\n");
	
		}
	}



}
