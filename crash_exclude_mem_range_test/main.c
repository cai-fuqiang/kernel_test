#include <stdio.h>
#include <stdlib.h>
#include "crash_mem.h"
extern int crash_exclude_mem_range(struct crash_mem *mem,
                            unsigned long long mstart, unsigned long long mend);

extern int crash_exclude_mem_range_my(struct crash_mem *mem,
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
			max_nr_ranges  * sizeof(struct range));
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
	for (i = 0; i < cmem->nr_ranges; i++) {
		printf("range[%d]:start(%lu)\tend(%lu)\n", 
				i,
				cmem->ranges[i].start,
				cmem->ranges[i].end);
	}
	return;
}


int test_exclude(struct crash_mem *cmem, u64 start, u64 end)
{
	static int i = 0;
	int ret = 0;
	printf("[%d]=====TEST start(%lu) end(%lu)=======\n",
			i, start, end);
	
	init_crash_mem(cmem);
	printf("test UPSTEAM code \n");
	ret = crash_exclude_mem_range(cmem, start, end);

	if (ret < 0) {
		goto exclude_err;

	}
	print_crash_mem(cmem);

	printf("test my code \n");
	init_crash_mem(cmem);
	ret = crash_exclude_mem_range_my(cmem, start, end);

	if (ret < 0) {
		goto exclude_err;
	}
	print_crash_mem(cmem);
end:
	i++;
	return ret;
exclude_err:
	printf("crash_exclude_mem_range error\n");
	goto end;
}
int main() {
	struct crash_mem *cmem;
	int ret;

	cmem = get_global_crash_mem();
	printf("the org arr\n");
	init_crash_mem(cmem);
	print_crash_mem(cmem);
	
	test_exclude(cmem, 0, 500);
	test_exclude(cmem, 500, 900);
	test_exclude(cmem, 500, 1000);
	test_exclude(cmem, 500, 1500);
	test_exclude(cmem, 500, 2000);
	test_exclude(cmem, 500, 2500);
	test_exclude(cmem, 1000, 2000);
	test_exclude(cmem, 0, 10000);
	test_exclude(cmem, 500, 10000);
	test_exclude(cmem, 3500, 3599);
	return 0;
}
