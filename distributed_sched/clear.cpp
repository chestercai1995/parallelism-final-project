
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

using namespace std;

sem_t ** stats_locks;
int main()
{
	stats_locks = new sem_t *[34];
    for(int i=0; i<34; i++)
    {
        stats_locks[i] = sem_open((char*) to_string(i).c_str(), O_RDWR, 0666, 1);
    }
		for(int i=0; i<34; i++)
		{
			sem_destroy(stats_locks[i]);
		}

	return 0;
}
