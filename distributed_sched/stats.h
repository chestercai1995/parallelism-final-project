
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdint.h>

//in us
#define RECORD_STAT_QUANTUM 500000
#define GLOBAL_SCHED_QUANTUM 500000

enum program_type {UNKNOWN, STREAMING, LG_MATMUL, SM_MATMUL, COMPUTE}; 
enum status {GOOD, BAD, WASTE};

int neighbours[34][4] = 
                 {  {4, 1, -1, -1},
                    {0, 5, -1, -1},
                    {8, 3, -1, -1},
                    {2, 9, -1, -1},
                    {10, 5, 0, -1},
                    {4, 11, 6, 1},
                    {5, 12, 7, -1},
                    {6, 13, 8, -1},
                    {7, 14, 9, 2},
                    {8, 15, 3, -1},
                    {16, 11, 4, -1},
                    {10, 17, 12, 5},
                    {11, 18, 13, 6},
                    {12, 19, 14, 7},
                    {13, 20, 15, 8},
                    {14, 21, 9, -1},
                    {22, 17, 10, -1},
                    {16, 23, 18, 11},
                    {17, 24, 19, 12},
                    {18, 25, 20, 13},
                    {19, 26, 21, 14},
                    {20, 27, 15, -1},
                    {28, 23, 16, -1},
                    {22, 29, 24, 17},
                    {23, 30, 25, 18},
                    {24, 31, 26, 19},
                    {25, 32, 27, 20},
                    {26, 33, 21, -1},
                    {29, 22, -1, -1},
                    {28, 30, 23, -1},
                    {29, 31, 24, -1},
                    {30, 32, 25, -1},
                    {31, 33, 26, -1},
                    {32, 27, -1, -1}    };

int num_neighbours[34] = 
                    {   2,
                        2,
                        2,
                        2,
                        3,
                        4,
                        3,
                        3,
                        4,
                        3,
                        3,
                        4,
                        4,
                        4,
                        4,
                        3,
                        3,
                        4,
                        4,
                        4,
                        4,
                        3,
                        3,
                        4,
                        4,
                        4,
                        4,
                        3,
                        2,
                        3,
                        3,
                        3,
                        3,
                        2 };


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

