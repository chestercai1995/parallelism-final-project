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
stats_struct **shm_ptrs;

//Global stats
stats_struct *core_stats;

int my_tile = 0;

/* =========================================== */


void *global_scheduler(int intr)
{
	int i = 0;
	for(; i < num_programs; i++){//see if there are any misidentified types
		core_stats[i].
	}
	for(i = 0; i < 34; i ++){//loot at each tile to see if each match makes sense
		if(core_stats[2 * i].type == STREAMING){
			if(core_stats[2*i + 1].type == STREAMING){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == SM_MATMUL){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == LG_MATMUL){
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == COMPUTE){
				core_stats[2 * i].stat = WASTE;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else{//unknown
			}
		}
		else if(core_stats[2 * i].type == SM_MATMUL){
			if(core_stats[2*i + 1].type == STREAMING){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == SM_MATMUL){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == LG_MATMUL){
				core_stats[2 * i].stat = WASTE;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == COMPUTE){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = WASTE;
			}
			else{//unknown
			}
		}
		else if(core_stats[2 * i].type == LG_MATMUL){
			if(core_stats[2*i + 1].type == STREAMING){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = BAD;
			}
			else if(core_stats[2*i + 1].type == SM_MATMUL){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = WASTE;
			}
			else if(core_stats[2*i + 1].type == LG_MATMUL){
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == COMPUTE){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else{//unknown
			}
		}
		else if(core_stats[2 * i].type == COMPUTE){
			if(core_stats[2*i + 1].type == STREAMING){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = WASTE;
			}
			else if(core_stats[2*i + 1].type == SM_MATMUL){
				core_stats[2 * i].stat = WASTE;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == LG_MATMUL){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == COMPUTE){
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else{//unknown
			}
		}
	}
	for(i = 0; i < num_programs; i++){
		if(core_stats[i].stat == BAD){
			int j;
            int found = -1;
			if(core_stats[i].type == STREAMING){
				for(j = 0; j < num_programs; j++){
				    if(core_stats[j].type == STREAMING 
                        || core_stats[j].type == SM_MATMUL){
                        if(core_stat[j].stat == GOOD)
                            continue;
                        found = j;
                        if(core_stats[j].stat == BAD){
                            break;
                        }
                    }	
				}
                if(found == -1){//did not find a suitable condidate to switch
                
                }
            }
            else if(core_stats[i].type == LG_MATMUL){
                int found = -1;
				for(j = 0; j < num_programs; j++){
				    if(core_stats[j].type == COMPUTE){
                        if(core_stats[j].stat == BAD){
                            found = j;
                            break;
                        }
                        else if(core_stats[j].stat == WASTE){
                            found = j;
                        }
                    }	
				}
                if(found == ){
                    
                }
            }
            else if(core_stats[i].type == COMPUTE){
				for(j = 0; j < num_programs; j++){
				    if(core_stats[j].type == STREAMING 
                        || core_stats[j].type == SM_MATMUL){
                        found = j;
                        if(core_stats[j].stat == BAD){
                            break;
                        }
                    }	
				}
            }
            //swap i with j
            pid_t temp;
		}
	}
	

	for(i = 0; i < num_programs; i++){
		if(shm_ptrs[i]!=NULL)
		{
			//printf("Reading from Proc %d :", i);
			memcpy(&core_stats[i], shm_ptrs[i], sizeof(stats_struct));
			//printf("%ld, %ld, %ld, %ld, %ld\n", core_stats[i].l2_cache_misses, core_stats[i].l2_cache_accesses, core_stats[i].num_instructions, core_stats[i].num_cycles, core_stats[i].num_ref_cycles);
		}
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
	int *shmids = new int[68];
	shm_ptrs = new stats_struct *[68];
	

	for(int i=0; i<num_programs; i++)
	{
		shm_ptrs[initial_affinity[i]] = (stats_struct *) get_shared_ptr( (char *) programs[i].c_str(), sizeof(stats_struct), SHM_W, &shmids[initial_affinity[i]]);
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
		
		for(int i=0; i<num_programs; i++)
		{
			detach_shared_mem(shm_ptrs[initial_affinity[i]]);
			destroy_shared_mem(&shmids[initial_affinity[i]]);
		}
		printf("Parent\n");
	
	}



}
