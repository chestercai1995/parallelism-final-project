#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "papi.h" 
#include "stats.h" 


int retval;
int EventSet0=PAPI_NULL;
char event_name0[PAPI_MAX_STR_LEN];
char event_name1[PAPI_MAX_STR_LEN];
char event_name2[PAPI_MAX_STR_LEN];
char event_name3[PAPI_MAX_STR_LEN];
long long values[4];

struct stats_struct *stats_ptrs;
core_write_struct * core_mapping;

void * timer_interrupt(int intr)
{
  if ( (retval = PAPI_stop(EventSet0, values) ) != PAPI_OK ){
        PAPI_perror("PAPI_stop");
        exit(-1);
  }


   
  //printf("Ending values for %s: %lld\n", event_name0,values[0]);
  //printf("Ending values for %s: %lld\n", event_name1,values[1]);
  //printf("Ending values for %s: %lld\n", event_name2,values[2]);
  //printf("Ending values for %s: %lld\n", event_name3,values[3]);
  
  if ( (retval = PAPI_start(EventSet0)) != PAPI_OK ){
	printf("failed here 0\n");
        PAPI_perror("PAPI_start");
        exit(-1);
  }


  //stats_struct * ptr = stats_ptrs + sizeof(stats_struct) * (*core_mapping);
  int64_t idx = core_mapping->core_write_id;
  //printf("Program at idx %d wrote stats\n", idx);
  stats_struct *ptr = &stats_ptrs[idx];
  

  ptr->l2_cache_misses = values[0];
  ptr->l2_cache_accesses = values[1];
  ptr->num_instructions = values[2];
  ptr->num_cycles = values[3];

  //printf("Written\n");

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
                sscanf(eventname, "%x", &real_event);
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

void setup_papi(){
  	int event_code0=PAPI_L2_TCM; /* By default monitor total instructions */
	int event_code1=PAPI_L2_TCA; /* By default monitor total instructions */
	int event_code2=PAPI_TOT_INS; 
	int event_code3=PAPI_TOT_CYC; 
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
	/* Now lets start counting */
	if ( (retval = PAPI_start(EventSet0)) != PAPI_OK ){
	      printf("failed here 0\n");
	      PAPI_perror("PAPI_start");
	      exit(-1);
	}

}
