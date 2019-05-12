#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>

#define RECORD_STAT_QUANTUM 100000
#define GLOBAL_SCHED_QUANTUM 100000

struct stats_struct
{
	int64_t l2_cache_misses;
	int64_t l2_cache_accesses;
	int64_t l2_cache_hits;
	int64_t num_instructions;
	int64_t num_cycles;
	int64_t num_ref_cycles;
	int32_t num_vol_ctxt_switches;
};
