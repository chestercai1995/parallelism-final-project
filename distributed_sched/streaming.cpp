#include "shm.h"
#include "sched.h"


//Global array; 400MB
#define array_size 100000000


int main(int argc, char *argv[])
{
	if(argc!=2) {
		printf("Agrument for multiples of runtime needed\n");
		return 1;
	}

	
	char * filename = argv[0];
	int shmid1;
	stats_ptrs = (stats_struct *) get_shared_ptr("stats_pt", sizeof(stats_struct)*68, SHM_W, &shmid1);
	
    int shmid2;
    core_mapping = (int *) get_shared_ptr(filename, sizeof(int), SHM_W, &shmid2);

    printf("%lx, %lx\n", stats_ptrs, core_mapping);
	
	setup_timer();
	setup_papi();
  

	int rt_mul = atoi(argv[1]);

	int rep_count = 10*rt_mul;
	for(int k=0; k<rep_count; k++)
	{
		//Setup phase
		uint32_t * global_array;
		global_array = new uint32_t[array_size];

		//printf("%u Starting setup\n", k);
		//fflush(stdout);
		for(int i=0; i<array_size; i++)
		{
			global_array[i] = 1;
		}
		
		//printf("%u Finished setup, starting processing\n", k);
		//fflush(stdout);

		//Processing phase
		
		for(int i=1; i<array_size; i++)
		{
			if(i%2==0)
				global_array[i] = global_array[i-1]*2 + 1;
			else
				global_array[i] = global_array[i-1] + 1;
		}
		
		//printf("%u Finished processing, terminating\n", k);
		//fflush(stdout);

		//Termination
		delete[] global_array;

	}
	

  detach_shared_mem(stats_ptrs);
  detach_shared_mem(core_mapping);
	
	
  PAPI_shutdown();
  exit(0);

}

