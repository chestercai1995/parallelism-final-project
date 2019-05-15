
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>

//in us
#define RECORD_STAT_QUANTUM 500000
#define GLOBAL_SCHED_QUANTUM 500000

enum program_type {UNKNOWN, STREAMING, LG_MATMUL, SM_MATMUL, COMPUTE}; 
enum status {GOOD, BAD, WASTE};

struct stats_struct
{
	int64_t l2_cache_misses;
	int64_t l2_cache_accesses;
	int64_t num_instructions;
	int64_t num_cycles;
	int64_t num_vol_ctxt_switches;


	int32_t pid;
    int32_t mapping_index;

	int32_t moved_recently;
    
    program_type type;
    status stat; 

    int32_t padding;

};

struct core_write_struct
{
    int64_t core_write_id;
    int64_t padding1;
    int64_t padding2;
    int64_t padding3;
    int64_t padding4;
    int64_t padding5;
    int64_t padding6;
    int64_t padding7;
};

