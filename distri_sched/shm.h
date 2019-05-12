#include <sched.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h>
#include <stdint.h>

void * get_shared_ptr(char *filename, uint32_t size, int shmflg, int *shmid)
{
	key_t key = ftok(filename, 65);
	*shmid = shmget(key, size, IPC_CREAT | 0666);
	if(shmid < 0)
	{
		printf("Unable to get shared pts\n");
	}
	return shmat(*shmid, NULL, shmflg);
} 

void detach_shared_mem(void *ptr)
{
	shmdt(ptr);
}

void destroy_shared_mem(int *shmid)
{
	shmctl(*shmid, IPC_RMID, NULL);
}
 

