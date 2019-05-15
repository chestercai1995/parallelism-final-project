#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>

//in us
#define RECORD_STAT_QUANTUM 100000
#define GLOBAL_SCHED_QUANTUM 100000
enum program_type {UNKNOWN, STREAMING, LG_MATMUL, SM_MATMUL, COMPUTE}; 
enum status {GOOD, BAD, WASTE};

struct stats_struct
{
	int64_t l2_cache_misses;
	int64_t l2_cache_accesses;
	int64_t num_instructions;
	int64_t num_cycles;
	int64_t num_ref_cycles;
	int64_t num_vol_ctxt_switches;

	pid_t pid;

	int32_t moved_recently;
	program_type type;
	status stat; //good do not need to move, bad need to move, waste is candidate for moving
};
