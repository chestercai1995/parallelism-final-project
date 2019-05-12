#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shm.h"
#include <semaphore.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
} while (0)

char sem_fn1[] = "my_sem1";
char sem_fn2[] = "my_sem2";

int
main(int argc, char *argv[])
{
cpu_set_t set;
int parentCPU, childCPU;
int nloops, j;
sem_t *semdes1;
sem_t *semdes2;

if (argc != 4) {
fprintf(stderr, "Usage: %s parent-cpu child-cpu num-loops\n",
argv[0]);
exit(EXIT_FAILURE);
}

char * filename = "new";
int shmid;
int *shm_ptr = (int *) get_shared_ptr(filename, sizeof(int)*4, SHM_W, &shmid);

parentCPU = atoi(argv[1]);
childCPU = atoi(argv[2]);
nloops = atoi(argv[3]);

semdes1 = sem_open(sem_fn1, O_CREAT, 0644, 0);
semdes2 = sem_open(sem_fn2, O_CREAT, 0644, 1);

CPU_ZERO(&set);

switch (fork()) {
case -1:            /* Error */
errExit("fork");

case 0:             /* Child */
CPU_SET(childCPU, &set);
semdes1 = sem_open(sem_fn1, 0);
semdes2 = sem_open(sem_fn2, 0);

if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
errExit("sched_setaffinity");



for (j = 0; j < nloops; j++)
{
	getppid();
	//printf("Writing to SHM\n");
	sem_wait(semdes2);
	shm_ptr[0] = j;
	shm_ptr[1] = j*10;
	shm_ptr[2] = j*100;
	shm_ptr[3] = j*1000;
	sem_post(semdes1);
}	

detach_shared_mem(shm_ptr);
exit(EXIT_SUCCESS);

default:            /* Parent */
CPU_SET(parentCPU, &set);

if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
errExit("sched_setaffinity");

for (j = 0; j < nloops; j++)
{
	//printf("Reading from SHM\n");
	sem_wait(semdes1);
	//printf("%d, %d, %d, %d\n", shm_ptr[0], shm_ptr[1], shm_ptr[2], shm_ptr[3]);
	sem_post(semdes2);
	getppid();

}
detach_shared_mem(shm_ptr);
wait(NULL);     /* Wait for child to terminate */
	destroy_shared_mem(&shmid);
exit(EXIT_SUCCESS);
}
}
