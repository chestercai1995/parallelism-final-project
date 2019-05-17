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

#define CYCLES_AFTER_MOVE 5
#define INIT_CYCLES 5

using namespace std;

/* =========================================== */
//Globals

int num_programs;
stats_struct *stats_ptrs;
core_write_struct **core_mapping;


//Global stats
stats_struct *core_stats;

//For shm
int shmid;
int *shmids;
int findNeighborIndex(int i){
    if(i%2){
        return i - 1;
    }
    else{
        return i + 1;
    }
    return -1;
}

/* =========================================== */
void swap_processes(int core_src, int core_dest)
{
    //printf("SSSSSSSSSSSSSSSSSSSSSSSSSs%lu\n", sizeof(stats_struct));
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

    src->moved_recently = CYCLES_AFTER_MOVE;
    dest->moved_recently = CYCLES_AFTER_MOVE;

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


    src->mrate_h3 = src->mrate_h2;
    src->mrate_h2 = src->mrate_h1;
    src->mrate_h1 = ((float)src->l2_cache_misses) / src->l2_cache_accesses;

    dest->mrate_h3 = dest->mrate_h2;
    dest->mrate_h2 = dest->mrate_h1;
    dest->mrate_h1 = ((float)dest->l2_cache_misses) / dest->l2_cache_accesses;

    //since your neighbor also observed a switch, need to update their histroy(roll forward one cycle as they have switched)
    stats_struct * src_neighbor =  &stats_ptrs[findNeighborIndex(i)];
    stats_struct * dest_neighbor =  &stats_ptrs[findNeighborIndex(j)];


    src_neighbor->mrate_h3 = src_neighbor->mrate_h2;
    src_neighbor->mrate_h2 = src_neighbor->mrate_h1;
    src_neighbor->mrate_h1 = ((float)src_neighbor->l2_cache_misses) / src_neighbor->l2_cache_accesses;

    dest_neighbor->mrate_h3 = dest_neighbor->mrate_h2;
    dest_neighbor->mrate_h2 = dest_neighbor->mrate_h1;
    dest_neighbor->mrate_h1 = ((float)dest_neighbor->l2_cache_misses) / dest_neighbor->l2_cache_accesses;

    //actually set affinity
    int set_val1 = sched_setaffinity(pid1, sizeof(set1), &set1);
    int set_val2 = sched_setaffinity(pid2, sizeof(set2), &set2);

    

    if(set_val1==-1 || set_val2==-1)
    {
        printf("Unable to swap threads\n");
    }
}

float abs(float f){
    if(f < 0){
        return -f;
    }    
    return f;
}

int compVar(int i){
    float v1, v2, v3, v4;
    v1 = stats_ptrs[i].l2_cache_misses / ((float)stats_ptrs[i].l2_cache_misses);
    v2 = stats_ptrs[i].mrate_h1;
    v3 = stats_ptrs[i].mrate_h2;
    v4 = stats_ptrs[i].mrate_h3;

    float d1, d2, d3, d4, d5, d6;
    d1 = abs(v1 - v2);
    d2 = abs(v1 - v3);
    d3 = abs(v1 - v4);
    d4 = abs(v2 - v3);
    d5 = abs(v2 - v4);
    d6 = abs(v3 - v4);

    float max = d1;
    if (max < d2){
        max = d2;
    }
    if (max < d3){
        max = d3;
    }
    if (max < d4){
        max = d4;
    }
    if (max < d5){
        max = d5;
    }
    if (max < d6){
        max = d6;
    }
    return max;
}

void *global_scheduler(int intr)
{

	int i = 0;
	for(; i < num_programs; i++){//see if there are any misidentified types
        //if(core_stats[i].l2_cache_accesses > 1500000){//larger than 1.5M, mark it as ST
        //    core_stats[i].type = STREAMING;
        //}
        //else if(core_stats[i].l2_cache_accesses > 130000){//larger than 130k marking it as LG
        //   core_stats[i].type = LG_MATMUL;
        //}
        //else if(core_stats[i].l2_cache_accesses > 10000){
        //    core_stats[i].type = SM_MATMUL;    
        //}
        //else{
        //    core_stats[i].type = COMPUTE;    
        //}
        if(core_stats[i].l2_cache_accesses < 10000){
            core_stats[i].type = COMPUTE;
        }
        else if(core_stats[i].mrate_h3 == 0){//with first 3 switches
            core_stats[i].type  = UNDETERMINED;
        }
        else{//based on variation, determine the type
            if(compVar(i)>0.20){
                //new phase
                core_stats[i].type  = UNDETERMINED;
            }
            if(compVar(i)>0.10){//lg
                core_stats[i].type  = LG_MATMUL;
            }
            if(compVar(i)>0.05){//sm
                core_stats[i].type  = SM_MATMUL;
            }
            if(compVar(i)>0.01){//st
                core_stats[i].type  = STREAMING;
            }
        }
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
                printf("identified core %d as bad\n",2*i);
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == COMPUTE){
                printf("identified core %d as waste\n",2*i);
				core_stats[2 * i].stat = WASTE;
				core_stats[2 * i + 1].stat = WASTE;
			}
			else if(core_stats[2*i + 1].type == UNDETERMINED){//unknown
				core_stats[2 * i].stat = BAD;
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
                printf("identified core %d as waste\n",2*i);
				core_stats[2 * i].stat = WASTE;
				core_stats[2 * i + 1].stat = WASTE;
			}
			else if(core_stats[2*i + 1].type == COMPUTE){
                printf("identified core %d as waste\n",2*i+1);
				core_stats[2 * i].stat = WASTE;
				core_stats[2 * i + 1].stat = WASTE;
			}
			else if(core_stats[2*i + 1].type == UNDETERMINED){//unknown
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
            }
			else{//unknown
			}
		}
		else if(core_stats[2 * i].type == LG_MATMUL){
			if(core_stats[2*i + 1].type == STREAMING){
                printf("identified core %d as bad\n",2*i+1);
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = BAD;
			}
			else if(core_stats[2*i + 1].type == SM_MATMUL){
                printf("identified core %d as waste\n",2*i+1);
				core_stats[2 * i].stat = WASTE;
				core_stats[2 * i + 1].stat = WASTE;
			}
			else if(core_stats[2*i + 1].type == LG_MATMUL){
                printf("identified core %d as bad\n",2*i);
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == COMPUTE){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == UNDETERMINED){//unknown
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = BAD;
            }
			else{//unknown
			}
		}
		else if(core_stats[2 * i].type == COMPUTE){
			if(core_stats[2*i + 1].type == STREAMING){
                printf("identified core %d as waste\n",2*i+1);
				core_stats[2 * i].stat = WASTE;
				core_stats[2 * i + 1].stat = WASTE;
			}
			else if(core_stats[2*i + 1].type == SM_MATMUL){
                printf("identified core %d as waste\n",2*i);
				core_stats[2 * i].stat = WASTE;
				core_stats[2 * i + 1].stat = WASTE;
			}
			else if(core_stats[2*i + 1].type == LG_MATMUL){
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == COMPUTE){
                printf("identified core %d as bad\n",2*i);
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == UNDETERMINED){//unknown
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
            }
			else{//unknown
			}
		}
		else if(core_stats[2 * i].type == UNDETERMINED){
			if(core_stats[2*i + 1].type == STREAMING){
                printf("identified core %d as waste\n",2*i+1);
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == SM_MATMUL){
                printf("identified core %d as waste\n",2*i);
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = BAD;
			}
			else if(core_stats[2*i + 1].type == LG_MATMUL){
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
			}
			else if(core_stats[2*i + 1].type == COMPUTE){
                printf("identified core %d as bad\n",2*i);
				core_stats[2 * i].stat = GOOD;
				core_stats[2 * i + 1].stat = BAD;
			}
			else if(core_stats[2*i + 1].type == UNDETERMINED){//unknown
				core_stats[2 * i].stat = BAD;
				core_stats[2 * i + 1].stat = GOOD;
            }
            else{
            }
		}
	}
    int found = -1;
	for(i = 0; i < num_programs; i++){//find a swap for each bad
        found = -1;
        if(core_stats[i].moved_recently)
        {
            printf("core %d has been moved recently, %d cycles left\n", i, core_stats[i].moved_recently);
            continue;
        }
	    if(core_stats[i].stat == BAD){
			int j;
			if(core_stats[i].type == STREAMING){//trying to find another streaming or small
				for(j = 0; j < num_programs; j++){
                    if(i == j){
                        continue;
                    }
				    if(core_stats[findNeighborIndex(j)].type == STREAMING 
                        || core_stats[findNeighborIndex(j)].type == SM_MATMUL){
                        if(core_stats[j].stat == GOOD)
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
				for(j = 0; j < num_programs; j++){//try finding a compute first
				    if(core_stats[findNeighborIndex(j)].type == COMPUTE){
                        if(core_stats[j].stat == BAD){
                            found = j;
                            break;
                        }
                        else if(core_stats[j].stat == WASTE){
                            found = j;
                        }
                    }	
				}
                if(found == -1){
				    for(j = 0; j < num_programs; j++){//try finding a compute first
				        if(core_stats[findNeighborIndex(j)].type == SM_MATMUL){
                            if(core_stats[j].stat == BAD){
                                found = j;
                                break;
                            }
                            else if(core_stats[j].stat == WASTE){
                                found = j;
                            }
                        }	
				    }
                }
                if(found == -1){
                }
            }
            else if(core_stats[i].type == COMPUTE){//if it's a bad computem, try finding a lg, if not, streaming, then small
                int found = -1;
				for(j = 0; j < num_programs; j++){//try finding a compute first
				    if(core_stats[findNeighborIndex(j)].type == LG_MATMUL){
                        if(core_stats[j].stat == BAD){
                            found = j;
                            break;
                        }
                        else if(core_stats[j].stat == WASTE){
                            found = j;
                        }
                    }	
				}
                if(found == -1){
				    for(j = 0; j < num_programs; j++){//try finding a compute first
				        if(core_stats[findNeighborIndex(j)].type == SM_MATMUL){
                            if(core_stats[j].stat == BAD){
                                found = j;
                                break;
                            }
                            else if(core_stats[j].stat == WASTE){
                                found = j;
                            }
                        }	
				    }
                }
                if(found == -1){
				    for(j = 0; j < num_programs; j++){//try finding a compute first
				        if(core_stats[findNeighborIndex(j)].type == STREAMING){
                            if(core_stats[j].stat == BAD){
                                found = j;
                                break;
                            }
                            else if(core_stats[j].stat == WASTE){
                                found = j;
                            }
                        }	
				    }
                }
                if(found == -1){
                }
            }//end of searching for a swap for a bad
            else if(core_stats[i].type == UNDETERMINED){//if it's a bad computem, try finding a lg, if not, streaming, then small
                int found = -1;
                if(core_stats[findNeighborIndex(i)].type == STREAMING || core_stats[findNeighborIndex(i)].type == LG_MATMUL){
                //if currently paired with an application that's large, try to find a compute, small or undetermined or STREAMING or Large in that order.
				    for(j = 0; j < num_programs; j++){//try finding a compute first
				        if(core_stats[findNeighborIndex(j)].type == COMPUTE){
                            if(core_stats[j].stat == BAD){
                                found = j;
                                break;
                            }
                            else if(core_stats[j].stat == WASTE){
                                found = j;
                            }
                        }	
				    }
                    if(found == -1){
				        for(j = 0; j < num_programs; j++){//try finding a compute first
				            if(core_stats[findNeighborIndex(j)].type == SM_MATMUL){
                                if(core_stats[j].stat == BAD){
                                    found = j;
                                    break;
                                }
                                else if(core_stats[j].stat == WASTE){
                                    found = j;
                                }
                            }	
				        }
                    }
                    if(found == -1){
				        for(j = 0; j < num_programs; j++){//try finding a compute first
				            if(core_stats[findNeighborIndex(j)].type == UNDETERMINED){
                                if(core_stats[j].stat == BAD){
                                    found = j;
                                    break;
                                }
                                else if(core_stats[j].stat == WASTE){
                                    found = j;
                                }
                            }	
				        }
                    }
                    if(found == -1){
				        for(j = 0; j < num_programs; j++){//try finding a compute first
				            if(core_stats[findNeighborIndex(j)].type == STREAMING){
                                if(core_stats[j].stat == BAD){
                                    found = j;
                                    break;
                                }
                                else if(core_stats[j].stat == WASTE){
                                    found = j;
                                }
                            }	
				        }
                    }
                    if(found == -1){
				        for(j = 0; j < num_programs; j++){//try finding a compute first
				            if(core_stats[findNeighborIndex(j)].type == LG_MATMUL){
                                if(core_stats[j].stat == BAD){
                                    found = j;
                                    break;
                                }
                                else if(core_stats[j].stat == WASTE){
                                    found = j;
                                }
                            }	
				        }
                    }
                    if(found == -1){//if there is no pair at all
                    //that sucks
                    }
                
                }
                else{
                //if currently not paried with an small application or don't know, pair it with LARGE, STREAMING, undetermined, small, compute, in that order
				    for(j = 0; j < num_programs; j++){//try finding a compute first
				        if(core_stats[findNeighborIndex(j)].type == LG_MATMUL){
                            if(core_stats[j].stat == BAD){
                                found = j;
                                break;
                            }
                            else if(core_stats[j].stat == WASTE){
                                found = j;
                            }
                        }	
				    }
                    if(found == -1){
				        for(j = 0; j < num_programs; j++){//try finding a compute first
				            if(core_stats[findNeighborIndex(j)].type == STREAMING){
                                if(core_stats[j].stat == BAD){
                                    found = j;
                                    break;
                                }
                                else if(core_stats[j].stat == WASTE){
                                    found = j;
                                }
                            }	
				        }
                    }
                    if(found == -1){
				        for(j = 0; j < num_programs; j++){//try finding a compute first
				            if(core_stats[findNeighborIndex(j)].type == UNDETERMINED){
                                if(core_stats[j].stat == BAD){
                                    found = j;
                                    break;
                                }
                                else if(core_stats[j].stat == WASTE){
                                    found = j;
                                }
                            }	
				        }
                    }
                    if(found == -1){
				        for(j = 0; j < num_programs; j++){//try finding a compute first
				            if(core_stats[findNeighborIndex(j)].type == SM_MATMUL){
                                if(core_stats[j].stat == BAD){
                                    found = j;
                                    break;
                                }
                                else if(core_stats[j].stat == WASTE){
                                    found = j;
                                }
                            }	
				        }
                    }
                    if(found == -1){
				        for(j = 0; j < num_programs; j++){//try finding a compute first
				            if(core_stats[findNeighborIndex(j)].type == LG_MATMUL){
                                if(core_stats[j].stat == BAD){
                                    found = j;
                                    break;
                                }
                                else if(core_stats[j].stat == WASTE){
                                    found = j;
                                }
                            }	
				        }
                    }
                    if(found == -1){//if there is no pair at all
                    //that sucks
                    }
                    
                }
		}
        if(found != -1){
            printf("*****************************\n");
            printf("trying to swap %d with %d\n", i, found);
            printf("*****************************\n");
            core_stats[i].stat=GOOD;
            core_stats[findNeighborIndex(i)].stat=GOOD;
            core_stats[findNeighborIndex(found)].stat=GOOD;
            core_stats[found].stat=GOOD;
            swap_processes(i, found);
        }
	}
    for(i = 0; i < num_programs; i++){
        if(stats_ptrs[i].moved_recently){
            stats_ptrs[i].moved_recently --;
        }
    }
	
	for(i = 0; i < num_programs; i++)
    {
        if(core_stats[i].type == COMPUTE){
            printf("compute");
        }
        else if(core_stats[i].type == LG_MATMUL){
            printf("lg_matmul");
        }
        else if(core_stats[i].type == SM_MATMUL){
            printf("sm_matmul");
        }
        else if(core_stats[i].type == STREAMING){
            printf("streaming");
        }
        printf(" core id: %d, l2 miss rate: %f", i, stats_ptrs[i].l2_cache_misses / ((float)stats_ptrs[i].l2_cache_accesses));
        printf(" MPKI: %f", (stats_ptrs[i].l2_cache_misses / ((float)stats_ptrs[i].num_instructions)) * 1000);
        printf(" ipc: %f\n", stats_ptrs[i].num_instructions/((float)stats_ptrs[i].num_cycles));
	}

	for(i = 0; i < num_programs; i++)
    {
		//if(shm_ptrs[i]!=NULL)
		//{
			//printf("Reading from Proc %d :", i);
			memcpy(&core_stats[i], &stats_ptrs[i], sizeof(stats_struct));
			//printf("%ld, %ld, %ld, %ld, %ld\n", core_stats[i].l2_cache_misses, core_stats[i].l2_cache_accesses, core_stats[i].num_instructions, core_stats[i].num_cycles, core_stats[i].num_ref_cycles);
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
        (&stats_ptrs[initial_affinity[i]])->moved_recently = INIT_CYCLES;
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
                printf("Completed app %d\n", done);
            }
        }
        
        //while (!child_scheds.empty())
        //{
        //    printf("Kill status: %d\n", kill(child_scheds.back(), SIGKILL));
        //    child_scheds.pop_back();
        //}
	    for(i = 0; i < num_programs; i++){//see if there are any misidentified types
            if(core_stats[i].l2_cache_accesses > 1500000){//larger than 1.5M, mark it as ST
                core_stats[i].type = STREAMING;
            }
            else if(core_stats[i].l2_cache_accesses > 130000){//larger than 130k marking it as LG
               core_stats[i].type = LG_MATMUL;
            }
            else if(core_stats[i].l2_cache_accesses > 10000){
                core_stats[i].type = SM_MATMUL;    
            }
            else{
                core_stats[i].type = COMPUTE;    
            }
	    }

        for(i = 0; i < 34; i++){//print the final mapping before quitting
            printf("on tile %d,", i);
            if(stats_ptrs[2 * i].type == COMPUTE){
                printf("compute");
            }
            else if(stats_ptrs[2 * i].type == LG_MATMUL){
                printf("lg_matmul");
            }
            else if(stats_ptrs[2 * i].type == SM_MATMUL){
                printf("sm_matmul");
            }
            else if(stats_ptrs[2 * i].type == STREAMING){
                printf("streaming");
            }
            printf(" is paired with ");
            if(stats_ptrs[2 * i + 1].type == COMPUTE){
                printf("compute");
            }
            else if(stats_ptrs[2 * i + 1].type == LG_MATMUL){
                printf("lg_matmul");
            }
            else if(stats_ptrs[2 * i + 1].type == SM_MATMUL){
                printf("sm_matmul");
            }
            else if(stats_ptrs[2 * i + 1].type == STREAMING){
                printf("streaming");
            }
            printf("\n");
        }
            
        detach_shared_mem(stats_ptrs);
        destroy_shared_mem(&shmid);

        for(int i=0; i<num_programs; i++)
        {
            detach_shared_mem(core_mapping[i]);
            destroy_shared_mem(&shmids[i]);
        }
	
	}



}
