#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#define cpu_init 0

int main(int argc, char** argv){
  if(argc < 2){
    printf("Please provide the program to run\n");
    exit(1);
  }
  char** new_arg = argv + 1;
  
  //setup perf counters
  struct perf_event_attr pe_l1access;
  struct perf_event_attr pe_l1miss;
  struct perf_event_attr pe_dtlbmiss;
  long long count;
  int fd_l1access;
  int fd_l1miss;
  int fd_dtlbmiss;

  memset(&pe_l1access, 0, sizeof(struct perf_event_attr));
  pe_l1access.type = PERF_TYPE_HW_CACHE;
  pe_l1access.size = sizeof(struct perf_event_attr);
  pe_l1access.config = PERF_COUNT_HW_CACHE_L1D | 
    (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16);
  pe_l1access.disabled = 1;
  pe_l1access.exclude_kernel = 1;
  pe_l1access.exclude_hv = 1;

  fd_l1access = perf_event_open(&pe_l1access, 0, -1, -1, 0);
  if(fd_l1access == -1){
    printf("1Error opening leader %llx, error code %s\n", 
        pe_l1access.config, strerror(errno));
    exit(1);
  }

  memset(&pe_l1miss, 0, sizeof(struct perf_event_attr));
  pe_l1miss.type = PERF_TYPE_HW_CACHE;
  pe_l1miss.size = sizeof(struct perf_event_attr);
  pe_l1miss.config = PERF_COUNT_HW_CACHE_L1D | 
    (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
  printf("config is %llx \n",pe_l1miss.config);
  pe_l1miss.disabled = 1;
  pe_l1miss.exclude_kernel = 1;
  pe_l1miss.exclude_hv = 1;

  fd_l1miss= perf_event_open(&pe_l1miss, 0, -1, fd_l1access, 0);
  if(fd_l1miss== -1){
    printf("2Error opening leader %llx, error code %s\n", 
        pe_l1miss.config, strerror(errno));
    exit(1);
  }

  memset(&pe_dtlbmiss, 0, sizeof(struct perf_event_attr));
  pe_dtlbmiss.type = PERF_TYPE_HW_CACHE;
  pe_dtlbmiss.size = sizeof(struct perf_event_attr);
  pe_dtlbmiss.config = PERF_COUNT_HW_CACHE_DTLB | 
    (PERF_COUNT_HW_CACHE_OP_READ << 8) | (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
  pe_dtlbmiss.disabled = 1;
  pe_dtlbmiss.exclude_kernel = 1;
  pe_dtlbmiss.exclude_hv = 1;

  fd_dtlbmiss= perf_event_open(&pe_dtlbmiss, 0, -1, fd_l1access, 0);
  if(fd_dtlbmiss == -1){
    printf("3Error opening leader %llx, error code %s\n", 
        pe_dtlbmiss.config, strerror(errno));
    exit(1);
  }
 

  //setup the timer interrupt
  
  //setaffinity
  cpu_set_t mask;
  CPU_ZERO(&mask);
  //TODO: init, set the process to a core at the beginning
  CPU_SET(cpu_init, &mask);
  
  int ret = sched_setaffinity(0, sizeof(mask), &mask);

  execv(argv[1], new_arg);
  return 0;
}
