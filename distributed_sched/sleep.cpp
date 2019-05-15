#include "shm.h"
#include "sched.h"
#include <unistd.h>

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
    core_mapping = (core_write_struct *) get_shared_ptr(filename, sizeof(core_write_struct), SHM_W, &shmid2);

    //printf("%lx, %lx\n", *(stats_ptrs), core_mapping);
	
	//setup_timer();
	setup_papi();
  

    sleep(10);	

  detach_shared_mem(stats_ptrs);
  detach_shared_mem(core_mapping);
	
	
  PAPI_shutdown();
  exit(0);

}

