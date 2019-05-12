/*
 * This example shows how to use PAPI_library_init, PAPI_create_eventset,
 * PAPI_add_event, * PAPI_start and PAPI_stop.  These 5 functions 
 * will allow a user to do most of the performance information gathering
 * that they would need. PAPI_read could also be used if you don't want
 * to stop the EventSet from running but only check the counts.
 *
 * Also, we will use PAPI_perror for * error information.
 *
 * In addition, a new call was created called PAPI_add_env_event
 * that allows a user to setup environment variable to read 
 * which event should be monitored this allows different events
 * to be monitored at runtime without recompiling, the syntax
 * is as follows:
 * PAPI_add_env_event(int *EventSet, int *Event, char *env_variable);
 * EventSet is the same as in PAPI_add_event
 * Event is the default event to monitor if the environment variable
 *       does not exist and differs from PAPI_add_event as it is
 *       a pointer.
 * env_varialbe is the name of the environment variable to look for
 *       the event code, this can be a name, number or hex, for example
 *       PAPI_L1_DCM could be defined in the environment variable as
 *       all of the following:  PAPI_L1_DCM, 0x80000000, or -2147483648
 *
 * To use only add_event you would change the calls to 
 * PAPI_add_env_event(int *EventSet, int *Event, char *env_variable);
 * to PAPI_add_event(int *EventSet, int Event);
 *
 * We will also use PAPI_event_code_to_name since the event may have
 * changed.
 * Author: Kevin London 
 * email:  london@cs.utk.edu
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "papi.h" /* This needs to be included anytime you use PAPI */
#define cycle 1000000

int PAPI_add_env_event(int *EventSet, int *Event, char *env_variable);


int main(){
  int retval,i;
  int EventSet0=PAPI_NULL;
  int event_code0=PAPI_L2_TCM; /* By default monitor total instructions */
  int event_code1=PAPI_L2_TCA; /* By default monitor total instructions */
  int event_code2=PAPI_L2_TCH; /* By default monitor total instructions */
  int event_code3=PAPI_L1_TCM; /* By default monitor total instructions */
  int event_code4=PAPI_L1_ICA; /* By default monitor total instructions */
  int event_code5=PAPI_L2_DCA; /* By default monitor total instructions */
  int event_code6=PAPI_TOT_INS; 
  int event_code7=PAPI_TOT_CYC; 
  int event_code8=PAPI_REF_CYC; 
  char errstring[PAPI_MAX_STR_LEN];
  char event_name0[PAPI_MAX_STR_LEN];
  char event_name1[PAPI_MAX_STR_LEN];
  char event_name2[PAPI_MAX_STR_LEN];
  char event_name3[PAPI_MAX_STR_LEN];
  char event_name4[PAPI_MAX_STR_LEN];
  char event_name5[PAPI_MAX_STR_LEN];
  char event_name6[PAPI_MAX_STR_LEN];
  char event_name7[PAPI_MAX_STR_LEN];
  char event_name8[PAPI_MAX_STR_LEN];
  float a[cycle],b[cycle],c[cycle];
  long long values[9];


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
  //if ( (retval=PAPI_add_env_event(&EventSet0, &event_code3, "PAPI_EVENT"))!=PAPI_OK){
  //      printf("failed here 3\n");
  //      PAPI_perror("PAPI_add_env_event");
  //      exit(-1);
  //}
  //if ( (retval=PAPI_add_env_event(&EventSet0, &event_code4, "PAPI_EVENT"))!=PAPI_OK){
  //      printf("failed here 4\n");
  //      PAPI_perror("PAPI_add_env_event");
  //      exit(-1);
  //}
  //if ( (retval=PAPI_add_env_event(&EventSet0, &event_code5, "PAPI_EVENT"))!=PAPI_OK){
  //      printf("failed here 5\n");
  //      PAPI_perror("PAPI_add_env_event");
  //      exit(-1);
  //}
  if ( (retval=PAPI_add_env_event(&EventSet0, &event_code6, "PAPI_EVENT"))!=PAPI_OK){
	printf("failed here 6\n");
        PAPI_perror("PAPI_add_env_event");
        exit(-1);
  }
  if ( (retval=PAPI_add_env_event(&EventSet0, &event_code7, "PAPI_EVENT"))!=PAPI_OK){
	printf("failed here 7\n");
        PAPI_perror("PAPI_add_env_event");
        exit(-1);
  }
  if ( (retval=PAPI_add_env_event(&EventSet0, &event_code8, "PAPI_EVENT"))!=PAPI_OK){
	printf("failed here 8\n");
        PAPI_perror("PAPI_add_env_event");
        exit(-1);
  }
  /* Now lets start counting */
  if ( (retval = PAPI_start(EventSet0)) != PAPI_OK ){
	printf("failed here 0\n");
        PAPI_perror("PAPI_start");
        exit(-1);
  }

  /* Some work to take up some time, the PAPI_start/PAPI_stop (and/or
   * PAPI_read) should surround what you want to monitor.
   */

  for ( i=0;i<cycle;i++){
        a[i] = b[i]-c[i];
        c[i] = a[i]*1.2;
  }
  if ( (retval = PAPI_stop(EventSet0, values) ) != PAPI_OK ){
        PAPI_perror("PAPI_stop");
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
  if ( (retval=PAPI_event_code_to_name( event_code6, event_name6))!=PAPI_OK){
        PAPI_perror("PAPI_event_code_to_name");   
        exit(-1);
  }
  if ( (retval=PAPI_event_code_to_name( event_code7, event_name7))!=PAPI_OK){
        PAPI_perror("PAPI_event_code_to_name");   
        exit(-1);
  }
  if ( (retval=PAPI_event_code_to_name( event_code8, event_name8))!=PAPI_OK){
        PAPI_perror("PAPI_event_code_to_name");   
        exit(-1);
  }


  printf("Ending values for %s: %lld\n", event_name0,values[0]);
  printf("Ending values for %s: %lld\n", event_name1,values[1]);
  printf("Ending values for %s: %lld\n", event_name2,values[2]);
  printf("Ending values for %s: %lld\n", event_name6,values[3]);
  printf("Ending values for %s: %lld\n", event_name7,values[4]);
  printf("Ending values for %s: %lld\n", event_name8,values[5]);
  //printf("Ending values for %s: %lld\n", event_name6,values[6]);
  //printf("Ending values for %s: %lld\n", event_name7,values[7]);
  //printf("Ending values for %s: %lld\n", event_name8,values[8]);
  /* Remove PAPI instrumentation, this is necessary on platforms
   * that need to release shared memory segments and is always
   * good practice.
   */
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

