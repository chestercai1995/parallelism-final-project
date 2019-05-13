
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>

//in us
#define RECORD_STAT_QUANTUM 100000
#define GLOBAL_SCHED_QUANTUM 100000

struct stats_struct
{
	int64_t l2_cache_misses;
	int64_t l2_cache_accesses;
	int64_t num_instructions;
	int64_t num_cycles;
	int64_t num_ref_cycles;
	int64_t num_vol_ctxt_switches;

	int32_t pid;

	int32_t updated;


};

struct tile_stats_struct
{
	int64_t l2_cache_misses_c0;
	int64_t l2_cache_accesses_c0;
	int64_t num_instructions_c0;
	int64_t num_cycles_c0;
	int64_t num_ref_cycles_c0;
	int64_t num_vol_ctxt_switches_c0;
	int64_t dummy_c0;

	int64_t l2_cache_misses_c1;
	int64_t l2_cache_accesses_c1;
	int64_t num_instructions_c1;
	int64_t num_cycles_c1;
	int64_t num_ref_cycles_c1;
	int64_t num_vol_ctxt_switches_c1;
	int64_t dummy_c1;

	int32_t pid_c0;
	int32_t pid_c1;

	int32_t updated_c0;
	int32_t updated_c1;


};
