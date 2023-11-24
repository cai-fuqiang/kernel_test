#include "crash_mem.h"

int crash_exclude_mem_range_my2(struct crash_mem *mem,
                            unsigned long long mstart, unsigned long long mend)
{
        int i, j;
        unsigned long long start, end, p_start, p_end;
        struct range temp_range = {0, 0};

        for (i = 0; i < mem->nr_ranges; i++) {
                start = mem->ranges[i].start;
                end = mem->ranges[i].end;
                p_start = mstart;
                p_end = mend;

                if (mstart > end || mend < start)
                        continue;

                /* Truncate any area outside of range */
                if (mstart < start)
                        p_start = start;
                if (mend > end)
                        p_end = end;

                /* Found completely overlapping range */
                if (p_start == start && p_end == end) {
                        mem->ranges[i].start = 0;
                        mem->ranges[i].end = 0;
                        if (i < mem->nr_ranges - 1) {
                                /* Shift rest of the ranges to left */
                                for (j = i; j < mem->nr_ranges - 1; j++) {
                                        mem->ranges[j].start =
                                                mem->ranges[j+1].start;
                                        mem->ranges[j].end =
                                                        mem->ranges[j+1].end;
                                }

                                /*
                                 * Continue to check if there are another overlapping ranges
                                 * from the current position because of shifting the above
                                 * mem ranges.
                                 */
                                i--;
                                mem->nr_ranges--;
                                continue;
                        }
                        mem->nr_ranges--;
                        return 0;
                }

                if (p_start > start && p_end < end) {
                        /* Split happened */
                        if (mem->nr_ranges == mem->max_nr_ranges)
                                return -ENOMEM;
                        /* Split original range */
                        mem->ranges[i].end = p_start - 1;
                        temp_range.start = p_end + 1;
                        temp_range.end = end;
                } else if (p_start != start)
                        mem->ranges[i].end = p_start - 1;
                else
                        mem->ranges[i].start = p_end + 1;
                break;
        }

        /* If a split happened, add the split to array */
        if (!temp_range.end)
                return 0;


        /* Location where new range should go */
        j = i + 1;
        if (j < mem->nr_ranges) {
                /* Move over all ranges one slot towards the end */
                for (i = mem->nr_ranges - 1; i >= j; i--)
                        mem->ranges[i + 1] = mem->ranges[i];
        }

        mem->ranges[j].start = temp_range.start;
        mem->ranges[j].end = temp_range.end;
        mem->nr_ranges++;
        return 0;
}


