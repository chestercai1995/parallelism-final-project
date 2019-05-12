#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "papi.h" /* This needs to be included anytime you use PAPI */
#include "shm.h"
#include "stats.h"

stats_struct *stats;

//Global array; 400MB
#define array_size 100000000
int PAPI_add_env_event(int *EventSet, int *Event, char *env_variable);
int retval;
int EventSet0=PAPI_NULL;
char event_name0[PAPI_MAX_STR_LEN];
char event_name1[PAPI_MAX_STR_LEN];
char event_name2[PAPI_MAX_STR_LEN];
char event_name3[PAPI_MAX_STR_LEN];
char event_name4[PAPI_MAX_STR_LEN];
char event_name5[PAPI_MAX_STR_LEN];
long long values[6];


void * timer_interrupt(int intr)
{
  if ( (retval = PAPI_stop(EventSet0, values) ) != PAPI_OK ){
        PAPI_perror("PAPI_stop");
        exit(-1);
  }
  printf("Ending values for %s: %lld\n", event_name0,values[0]);
  printf("Ending values for %s: %lld\n", event_name1,values[1]);
  printf("Ending values for %s: %lld\n", event_name2,values[2]);
  printf("Ending values for %s: %lld\n", event_name3,values[3]);
  printf("Ending values for %s: %lld\n", event_name4,values[4]);
  printf("Ending values for %s: %lld\n", event_name5,values[5]);
  if ( (retval = PAPI_start(EventSet0)) != PAPI_OK ){
	printf("failed here 0\n");
        PAPI_perror("PAPI_start");
        exit(-1);
  }

  return NULL;
}

void setup_timer()
{
	struct timeval value = {1, 0};
	struct timeval interval = {0, RECORD_STAT_QUANTUM};
	struct itimerval timer = {interval, value};
	
	signal(SIGPROF, (__sighandler_t) timer_interrupt);
	setitimer(ITIMER_PROF, &timer, 0);
	return;
}

int main(int argc, char *argv[])
{
	if(argc!=2) {
		printf("Agrument for multiples of runtime needed\n");
		return 1;
	}

	stats = (stats_struct *) calloc(1, sizeof(stats_struct));
	setup_timer();
  
	int i;
  	int event_code0=PAPI_L2_TCM; /* By default monitor total instructions */
	int event_code1=PAPI_L2_TCA; /* By default monitor total instructions */
	int event_code2=PAPI_L2_TCH; /* By default monitor total instructions */
	int event_code3=PAPI_TOT_INS; 
	int event_code4=PAPI_TOT_CYC; 
	int event_code5=PAPI_REF_CYC; 
	char errstring[PAPI_MAX_STR_LEN];
 

  /* This initializes the library and checks the version number of the
   * header file, to the version of the library, if these don't match
   * then it is likely that PAPI won't work correctly.  
   */
  if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT ){
	/* This call loads up what the error means into errstring 
	 * if retval == PAPI_ESYS then it might be beneficial
 	 * to call perror as well to see what system call failed
	 */
	PAPI_perror("PAPI_library_init");
	exit(-1);
  }
  /* Create space for the EventSet */
  if ( (retval=PAPI_create_eventset( &EventSet0 ))!=PAPI_OK){
	printf("failed here 0\n");
	PAPI_perror(errstring);
        exit(-1);
  }

  /*  After this call if the environment variable PAPI_EVENT is set,
   *  event_code may contain something different than total instructions.
   */
  if ( (retval=PAPI_add_env_event(&EventSet0, &event_code0, "PAPI_EVENT"))!=PAPI_OK){
	printf("failed here 0\n");
        PAPI_perror("PAPI_add_env_event");
        exit(-1);
  }
  if ( (retval=PAPI_add_env_event(&EventSet0, &event_code1, "PAPI_EVENT"))!=PAPI_OK){
	printf("failed here 1\n");
        PAPI_perror("PAPI_add_env_event");
        exit(-1);
  }
  if ( (retval=PAPI_add_env_event(&EventSet0, &event_code2, "PAPI_EVENT"))!=PAPI_OK){
	printf("failed here 2\n");
        PAPI_perror("PAPI_add_env_event");
        exit(-1);
  }
  if ( (retval=PAPI_add_env_event(&EventSet0, &event_code3, "PAPI_EVENT"))!=PAPI_OK){
	printf("failed here 6\n");
        PAPI_perror("PAPI_add_env_event");
        exit(-1);
  }
  if ( (retval=PAPI_add_env_event(&EventSet0, &event_code4, "PAPI_EVENT"))!=PAPI_OK){
	printf("failed here 7\n");
        PAPI_perror("PAPI_add_env_event");
        exit(-1);
  }
  if ( (retval=PAPI_add_env_event(&EventSet0, &event_code5, "PAPI_EVENT"))!=PAPI_OK){
	printf("failed here 8\n");
        PAPI_perror("PAPI_add_env_event");
        exit(-1);
  }
  if ( (retval=PAPI_event_code_to_name( event_code0, event_name0))!=PAPI_OK){
        PAPI_perror("PAPI_event_code_to_name");   
        exit(-1);
  }
  if ( (retval=PAPI_event_code_to_name( event_code1, event_name1))!=PAPI_OK){
        PAPI_perror("PAPI_event_code_to_name");   
        exit(-1);
  }
  if ( (retval=PAPI_event_code_to_name( event_code2, event_name2))!=PAPI_OK){
        PAPI_perror("PAPI_event_code_to_name");   
        exit(-1);
  }
  if ( (retval=PAPI_event_code_to_name( event_code3, event_name3))!=PAPI_OK){
        PAPI_perror("PAPI_event_code_to_name");   
        exit(-1);
  }
  if ( (retval=PAPI_event_code_to_name( event_code4, event_name4))!=PAPI_OK){
        PAPI_perror("PAPI_event_code_to_name");   
        exit(-1);
  }
  if ( (retval=PAPI_event_code_to_name( event_code5, event_name5))!=PAPI_OK){
        PAPI_perror("PAPI_event_code_to_name");   
        exit(-1);
  }
  /* Now lets start counting */
  if ( (retval = PAPI_start(EventSet0)) != PAPI_OK ){
	printf("failed here 0\n");
        PAPI_perror("PAPI_start");
        exit(-1);
  }
  //TODO work to do




  /* Remove PAPI instrumentation, this is necessary on platforms
   * that need to release shared memory segments and is always
   * good practice.
   */

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

int PAPI_add_env_event(int *EventSet, int *EventCode, char *env_variable){
  int real_event=*EventCode;
  char *eventname;
  int retval;
 
  if ( env_variable != NULL ){
    if ( (eventname=getenv(env_variable)) ) {
        if ( eventname[0] == 'P' ) {  /* Use the PAPI name */
           retval=PAPI_event_name_to_code(eventname, &real_event );
           if ( retval != PAPI_OK ) real_event = *EventCode;
        }
        else{
           if ( strlen(eventname)>1 && eventname[1]=='x')
                sscanf(eventname, "%#x", &real_event);
           else
               real_event = atoi(eventname);
        }
    }
  }
  if ( (retval = PAPI_add_event( *EventSet, real_event))!= PAPI_OK ){
        if ( real_event != *EventCode ) {
                if ( (retval = PAPI_add_event( *EventSet, *EventCode)) == PAPI_OK){
                        real_event = *EventCode;
                }
        }
  }
  *EventCode = real_event;
  return retval;
}
