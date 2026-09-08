#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>

static int __init pause_test_init(void)
{
    u64 start, end, diff;
    u64 total_us, total_ms, avg_ns;
    int i,j,k;
    const int iterations = 1024 * 1024 * 128;
    unsigned long flags;

    pr_info("Starting pause test: %d iterations\n", iterations);

    start = ktime_get_ns();

    local_irq_save(flags);

    for (i = 0; i < iterations; i++) {
        __asm__ volatile ("pause" ::: "memory");
    }
    local_irq_restore(flags);
    end = ktime_get_ns();
    diff = end - start;

    /* 全部使用整数运算，避免浮点指令 */
    total_us = diff / 1000;          /* 微秒 */
    total_ms = diff / 1000000;       /* 毫秒 */
    avg_ns = diff / iterations;      /* 平均纳秒 */

    pr_info("Total time for %d pause instructions: %llu ns (%llu us, %llu ms)\n",
            iterations, diff, total_us, total_ms);
    pr_info("Average time per pause: %llu ns\n", avg_ns);

    return 0;
}

static void __exit pause_test_exit(void)
{
    pr_info("pause test module unloaded\n");
}

module_init(pause_test_init);
module_exit(pause_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Measure 10000 pause instructions with -O0 (no FPU)");
