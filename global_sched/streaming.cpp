#include "shm.h"
#include "sched.h"

stats_struct *stats;

//Global array; 400MB
#define array_size 100000000



int main(int argc, char *argv[])
{
	if(argc!=2) {
		printf("Agrument for multiples of runtime needed\n");
		return 1;
	}

	stats = (stats_struct *) calloc(1, sizeof(stats_struct));
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
	char * filename = (char*) "bin/streaming";
	int shmid;
	int *shm_ptr = (int *) get_shared_ptr(filename, sizeof(int)*4, SHM_RDONLY, &shmid);

	printf("Reading from SHM\n");
	printf("%d, %d, %d, %d\n", shm_ptr[0], shm_ptr[1], shm_ptr[2], shm_ptr[3]);

	printf("Done reading\n");
	detach_shared_mem(shm_ptr);
	destroy_shared_mem(&shmid);
	
  PAPI_shutdown();
  exit(0);

}

