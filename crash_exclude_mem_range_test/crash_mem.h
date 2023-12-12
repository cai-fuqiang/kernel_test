#ifndef __CRASH_MEM__
#define __CRASH_MEM__

typedef unsigned long u64;
typedef unsigned char bool;

struct range {
       u64 start;
       u64 end;
};

struct crash_mem {
	unsigned int max_nr_ranges;
	unsigned int nr_ranges;
	struct range ranges[];
};

#define RANGE_ARR_NUM	2
#define EX_TIME		1

#define AVOID_OVER	16

#define EACH_RANGE_SIZE	1000

#define ENOMEM		1
#endif
