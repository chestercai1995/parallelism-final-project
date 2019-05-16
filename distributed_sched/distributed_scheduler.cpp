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
#include <semaphore.h>
#include <fcntl.h>

#include "shm.h"
#include "stats.h"

using namespace std;

/* =========================================== */
//Globals

int num_programs;

stats_struct *stats_ptrs;

core_write_struct **core_mapping;

sem_t ** stats_locks;

int shmid;
int *shmids;

//Global stats
stats_struct *core_stats;

int my_tile = 0;

int global_cnt = 0;

int startup_latency = STARTUP_LATENCY;

/* =========================================== */

void swap_processes(int core_src, int core_dest)
{
    printf("SSSSSSSSSSSSSSSSSSSSSSSSSs%lu\n", sizeof(stats_struct));
    int i = core_src;
    int j = core_dest;
    //swap
    //swap pids
    stats_struct * src =  &stats_ptrs[i];
    stats_struct * dest =  &stats_ptrs[j];

    int32_t pid1 = src->pid;
    int32_t pid2 = dest->pid;

    int32_t map1 = src->mapping_index;
    int32_t map2 = dest->mapping_index;

    src->pid = pid2;
    dest->pid = pid1;
    
    src->mapping_index = map2;
    dest->mapping_index = map1;
    
	src->moved_recently = SWAP_INERTIA;
    dest->moved_recently = SWAP_INERTIA;
	
	src->past_l2_mr = (float) (dest->l2_cache_misses)/(float) (dest->l2_cache_accesses);
    dest->past_l2_mr = (float) (src->l2_cache_misses)/(float) (src->l2_cache_accesses);

    int32_t core1 = core_mapping[map1]->core_write_id;
    int32_t core2 = core_mapping[map2]->core_write_id;
    
    core_mapping[map1]->core_write_id = core2;
    core_mapping[map2]->core_write_id = core1;

    cpu_set_t set1;
    cpu_set_t set2;

    CPU_ZERO(&set1);
    CPU_ZERO(&set2);

    CPU_SET(core2, &set1);
    CPU_SET(core1, &set2);

    int set_val1 = sched_setaffinity(pid1, sizeof(set1), &set1);
    int set_val2 = sched_setaffinity(pid2, sizeof(set2), &set2);

    if(set_val1==-1 || set_val2==-1)
    {
        printf("Unable to swap threads\n");
    }
         

}

inline program_type update_type(stats_struct *ptr)
{
  	if(ptr->l2_cache_accesses > 1500000){//larger than 1.5M, mark it as ST
		ptr->type = STREAMING;
		return STREAMING;
	}
	else if(ptr->l2_cache_accesses > 130000){//larger than 130k marking it as LG
	   ptr->type = LG_MATMUL;
		return LG_MATMUL;
	}
	else if(ptr->l2_cache_accesses > 10000){
		ptr->type = SM_MATMUL;    
		return SM_MATMUL;
	}
	else{
		ptr->type = COMPUTE;    
		return COMPUTE;
	}
}

//0 nothing
//1 swap 1
//2 swap 2
//3 maybe swap 1
//4 maybe swap 2
inline int is_good_mapping(program_type prog1, program_type prog2)
{
	if(prog1 == STREAMING && prog2 == STREAMING) return 0;
	else if(prog1 == STREAMING && prog2 == SM_MATMUL) return 0;
	else if(prog1 == STREAMING && prog2 == LG_MATMUL) return 1;
	else if(prog1 == STREAMING && prog2 == COMPUTE) return 3;
	else if(prog1 == SM_MATMUL && prog2 == STREAMING) return 0;
	else if(prog1 == SM_MATMUL && prog2 == SM_MATMUL) return 3;
	else if(prog1 == SM_MATMUL && prog2 == LG_MATMUL) return 3;
	else if(prog1 == SM_MATMUL && prog2 == COMPUTE) return 4;
	else if(prog1 == LG_MATMUL && prog2 == STREAMING) return 2;
	else if(prog1 == LG_MATMUL && prog2 == SM_MATMUL) return 4;
	else if(prog1 == LG_MATMUL && prog2 == LG_MATMUL) return 1;
	else if(prog1 == LG_MATMUL && prog2 == COMPUTE) return 0;
	else if(prog1 == COMPUTE && prog2 == STREAMING) return 4;
	else if(prog1 == COMPUTE && prog2 == SM_MATMUL) return 3;
	else if(prog1 == COMPUTE && prog2 == LG_MATMUL) return 0;
	else if(prog1 == COMPUTE && prog2 == COMPUTE) return 1;
	else return 0;		
	
	
}

void *distributed_mapper(int intr)
{

    if((my_tile==0 || my_tile==1) && startup_latency==0)
    {
        
        printf("Trying to acquire locks\n");
        
        int my_lock = sem_trywait(stats_locks[my_tile]);
       
        int n_locks = 0;
        for(int i=0; i<num_neighbours[my_tile]; i++)
        {
            int neighbouring_tile = neighbours[my_tile][i]; 
            n_locks = n_locks || sem_trywait(stats_locks[neighbouring_tile]);
        }

		//Can optimize here
        if((n_locks||my_lock)==0)
        {
            printf("%d acquired locks:", my_tile);
            for(int i=0; i<num_neighbours[my_tile]; i++)
            {
                int neighbouring_tile = neighbours[my_tile][i]; 
                printf("%d,", neighbouring_tile);
            }
            printf("\n");

            // Now that we've acquired locks, look at the program classes for 
            // the neighbouring programs. If a good match is found, swap.
           
			//update my type
			int c1 = 2*my_tile;
			int c2 = 2*my_tile+1;
			stats_struct * my_stats1 = &stats_ptrs[c1];
			stats_struct * my_stats2 = &stats_ptrs[c2];
			program_type prog1 = update_type(my_stats1);
			program_type prog2 = update_type(my_stats2);

			if(!(my_stats1->moved_recently || my_stats2->moved_recently))
			{

			//Only actively look to swap if you're not doing so well
			//Later, add potential. Look to swap if potential is good
		
			printf("My types %d %d\n", prog1, prog2);

			int mapping_type = is_good_mapping(prog1, prog2);
			printf("Mapping %d\n", mapping_type);
			
			if(mapping_type==1 || mapping_type==2)
			{
				int swap_candidate = (mapping_type+1)%2;
				printf("%d, %d\n", my_tile, num_neighbours[my_tile]);
				for(int i=0; i<num_neighbours[my_tile]; i++)
				{
					int neighbouring_tile = neighbours[my_tile][i];
					int c1_n = 2*neighbouring_tile;
					int c2_n = 2*neighbouring_tile+1;
					program_type prog1_n = (&stats_ptrs[c1_n])->type;
					program_type prog2_n = (&stats_ptrs[c2_n])->type;
		
					if(prog1_n==UNKNOWN || prog2_n==UNKNOWN) continue;
					printf("Neighbour %d %d types %d %d\n", c1_n, c2_n, prog1_n, prog2_n);
					
					int swap1_1 = is_good_mapping(prog1_n, prog2);
					int swap1_2 = is_good_mapping(prog2_n, prog1);

					int swap2_1 = is_good_mapping(prog1_n, prog1);
					int swap2_2 = is_good_mapping(prog2_n, prog2);

					printf("Mapping pot %d %d %d %d\n", swap1_1, swap1_2, swap2_1, swap2_2);
					
					if(swap1_1==0 || swap1_2==0)
					{
						if(swap_candidate==0)
						{
							printf("Swapping %d and %d\n", c1, c1_n);
							swap_processes(c1, c1_n);
							break;
						}
						else
						{
							printf("Swapping %d and %d\n", c2, c2_n);
							swap_processes(c2, c2_n);
							break;
						}
					}
					else if(swap2_1==0 || swap2_2==0)
					{
						if(swap_candidate==0)
						{
							printf("Swapping %d and %d\n", c1, c2_n);
							swap_processes(c1, c2_n);
							break;
						}
						else
						{
							printf("Swapping %d and %d\n", c2, c1_n);
							swap_processes(c2, c1_n);
							break;
						}
					}
					

				}
			}
			}
			
            sem_post(stats_locks[my_tile]);        
            for(int i=0; i<num_neighbours[my_tile]; i++)
            {
                int neighbouring_tile = neighbours[my_tile][i];
                sem_post(stats_locks[neighbouring_tile]);
            }
            printf("released locks\n");
        }
    }
	else
	{
		startup_latency--;
	}

    int i = 2*my_tile; 

    if(my_tile==1 || my_tile==0){

		printf("Reading from Proc %d :", i);
		stats_struct * ptr = &stats_ptrs[i];
		printf("%ld, %ld, %ld, %ld\n", ptr->l2_cache_misses, ptr->l2_cache_accesses, ptr->num_instructions, ptr->num_cycles);
	    
        i = 2*my_tile+1;
		
        printf("Reading from Proc %d :", i);
		ptr = &stats_ptrs[i];
		printf("%ld, %ld, %ld, %ld\n", ptr->l2_cache_misses, ptr->l2_cache_accesses, ptr->num_instructions, ptr->num_cycles);

    
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
	stats_locks = new sem_t *[34];

	stats_ptrs = (stats_struct *) get_shared_ptr( "stats_pt", sizeof(stats_struct)*68, SHM_W, &shmid);

    //stats_struct tryi = stats_ptrs[3];

	for(int i=0; i<68; i++)
	{
        for(int k=0; k<num_programs; k++) 
        {
            if(initial_affinity[k] == i)
            {
                core_mapping[k] = (core_write_struct *) get_shared_ptr( (char *) programs[k].c_str(), sizeof(core_write_struct), SHM_W, &shmids[k]);
                core_mapping[k]->core_write_id = i;
                //Add pid to stats_struct
                break;
            }
        }
	}
    /* 
    for(int k=num_programs; k<68; k++)
    {
        core_mapping[k] = (core_write_struct *) get_shared_ptr_noid(sizeof(core_write_struct), SHM_W, &shmids[k]);
        core_mapping[k]->core_write_id = -1;
    }
    */
    for(int i=0; i<34; i++)
    {
        stats_locks[i] = sem_open((char*) to_string(i).c_str(), O_CREAT, 0666, 1);
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
        (&stats_ptrs[initial_affinity[i]])->moved_recently = SWAP_INERTIA;
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
                //bool created = false;
                for(int k=0; k<num_programs; k++) 
                {
                    if(initial_affinity[k] == i)
                    {
                        core_mapping[k] = (core_write_struct *) get_shared_ptr( (char *) programs[k].c_str(), sizeof(core_write_struct), SHM_W, &shmids[k]);
                        core_mapping[k]->core_write_id = i;
                        //created= true;
                        break;
                    }
                }
                //semaphores
            }
            /*
            for(int k=num_programs; k<68; k++)
            {
                core_mapping[k] = (core_write_struct *) get_shared_ptr_noid(sizeof(core_write_struct), SHM_W, &shmids[k]);
                core_mapping[k]->core_write_id = -1;
            }
            */
            for(int i=0; i<34; i++)
            {
                stats_locks[i] = sem_open((char*) to_string(i).c_str(), O_CREAT, 0666, 1);
            }

			
            my_tile = i+1;
			printf("Child sched %d\n", my_tile);
			struct timeval value = {1, 0};
			struct timeval interval = {0, GLOBAL_SCHED_QUANTUM};
			struct itimerval timer = {interval, value};
			
			signal(SIGALRM, (__sighandler_t) distributed_mapper);
			setitimer(ITIMER_REAL, &timer, 0);


			while(1) pause();
		}
		else
		{
			
			printf("Parent sched %d\n", my_tile);
			struct timeval value = {1, 0};
			struct timeval interval = {0, GLOBAL_SCHED_QUANTUM};
			struct itimerval timer = {interval, value};
			
			signal(SIGALRM, (__sighandler_t) distributed_mapper);
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
