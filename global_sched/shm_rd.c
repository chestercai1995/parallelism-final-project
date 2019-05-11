#include <stdio.h>
#include <stdint.h>
#include "shm.h"

int main()
{
	char * filename = "new";
	int shmid;
	int *shm_ptr = (int *) get_shared_ptr(filename, sizeof(int)*4, SHM_RDONLY, &shmid);

	printf("Reading from SHM\n");
	printf("%d, %d, %d, %d\n", shm_ptr[0], shm_ptr[1], shm_ptr[2], shm_ptr[3]);

	printf("Done reading\n");
	detach_shared_mem(shm_ptr);
	destroy_shared_mem(&shmid);
}
