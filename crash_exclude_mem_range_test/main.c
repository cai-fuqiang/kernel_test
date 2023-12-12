#include <stdio.h>
#include <stdlib.h>
#include "crash_mem.h"
extern int crash_exclude_mem_range(struct crash_mem *mem,
                            unsigned long long mstart, unsigned long long mend);

extern int crash_exclude_mem_range_my(struct crash_mem *mem,
                            unsigned long long mstart, unsigned long long mend);
extern int crash_exclude_mem_range_out_of_bounds(struct crash_mem *mem,
                            unsigned long long mstart, unsigned long long mend);
extern int crash_exclude_mem_range_new(struct crash_mem *mem,
                            unsigned long long mstart, unsigned long long mend);
struct crash_mem * get_global_crash_mem()
{
	static struct crash_mem *cmem = NULL;
	int i = 0;
	unsigned int max_nr_ranges = 0;
	if (cmem)
		return cmem;

	max_nr_ranges = RANGE_ARR_NUM + EX_TIME;
	cmem = (struct crash_mem*) malloc(sizeof(struct crash_mem) + 
			(max_nr_ranges + AVOID_OVER)  * sizeof(struct range));
	cmem->max_nr_ranges = max_nr_ranges;

	if (!cmem) {
		printf("cannot malloc memory, abort !\n");
		abort();
	}
	return cmem;
}

void init_crash_mem(struct crash_mem *cmem)
{
	int i = 0;

	cmem->nr_ranges = 0;
        for (i = 0; i < cmem->max_nr_ranges - EX_TIME ; i++) {
                cmem->ranges[i].start =
                        i == 0 ? 1 : cmem->ranges[i-1].end + 1;
                cmem->ranges[i].end = cmem->ranges[i].start
                                      + EACH_RANGE_SIZE - 1;
                cmem->nr_ranges++;
        }
	return;
}

void print_crash_mem(struct crash_mem *cmem)
{
	int i = 0;
	printf("nr_ranges:(%u)\n", cmem->nr_ranges);
	printf("max_nr_ranges:(%u)\n", cmem->max_nr_ranges);
	if (cmem->nr_ranges > cmem->max_nr_ranges) {
		printf("when print, check result is OUT OF BOUNDS: \n"
			"\tcmem->nr_ranges(%d) > cmem->max_nr_ranges(%d)\n",
			cmem->nr_ranges, cmem->max_nr_ranges);
	}
	for (i = 0; i < cmem->nr_ranges; i++) {
		printf("range[%d]:start(%lu)\tend(%lu)", 
				i,
				cmem->ranges[i].start,
				cmem->ranges[i].end);
		if (i == cmem->max_nr_ranges)
			printf("  OUT OF BOUND \n");
		else
			printf("\n");

	}
	return;
}


int test_exclude(struct crash_mem *cmem, int ex_time, u64 start[], u64 end[])
{
	static int i = 0;
	int j = 0;
	int ret = 0;
	u64 p_start, p_end;
	printf("[%d]=====TEST=======\n", i);
	
	init_crash_mem(cmem);
	printf("test UPSTEAM code \n");
	for (j = 0; j < ex_time; j++) {
		p_start = start[j];
		p_end = end[j];
		ret = crash_exclude_mem_range(cmem, p_start, p_end);

		if (ret < 0) {
			printf(" crash_exclude_mem_range ret = %d\n", ret);
		}
	}
	print_crash_mem(cmem);

	init_crash_mem(cmem);
	printf("test my code \n");
	for (j = 0; j < ex_time; j++) {
		p_start = start[j];
		p_end = end[j];
		ret = crash_exclude_mem_range_out_of_bounds(cmem, p_start, p_end);

		if (ret < 0) {
			printf("crash_exclude_mem_range ret = %d\n", ret);
		}
	}
	print_crash_mem(cmem);
end:
	i++;
	return ret;
}
int main() {
	struct crash_mem *cmem;
	int ret;
	u64 start[] = {100, 300};
	u64 end[] = {200, 400};

	cmem = get_global_crash_mem();
	printf("the org arr\n");
	init_crash_mem(cmem);
	print_crash_mem(cmem);
	
	//test_exclude(cmem, 500, 400);
	test_exclude(cmem, 2, start, end);

	//start[0] = 0;
	//end[0] = 2001;
	//test_exclude(cmem, 1, start, end);
	return 0;
}
