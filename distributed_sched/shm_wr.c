#include <stdio.h>
#include <stdint.h>
#include "shm.h"

int main()
{
	char * filename = "new";
	int shmid;
	int *shm_ptr = (int *) get_shared_ptr(filename, sizeof(int)*4, SHM_W, &shmid);

	printf("Writing to SHM\n");
	shm_ptr[0] = 10;
	shm_ptr[1] = 20;
	shm_ptr[2] = 30;
	shm_ptr[3] = 40;

	printf("Done writing\n");
	detach_shared_mem(shm_ptr);	
}
