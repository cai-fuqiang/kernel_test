#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/timekeeping.h>
#include <linux/slab.h>
#include <linux/err.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kernel spinlock performance test");

/* 模块参数 */
static int nr_threads = 4;
static unsigned long loops_per_thread = 100000;
module_param(nr_threads, int, 0644);
MODULE_PARM_DESC(nr_threads, "Number of kernel threads");
module_param(loops_per_thread, ulong, 0644);
MODULE_PARM_DESC(loops_per_thread, "Number of loops per thread");

static spinlock_t test_lock;
static unsigned long shared_counter = 0;
static struct task_struct **threads;
static int thread_count;

static int spinlock_thread_fn(void *data)
{
    unsigned long i;
    for (i = 0; i < loops_per_thread; i++) {
        spin_lock(&test_lock);
        shared_counter++;
        spin_unlock(&test_lock);
    }
    return 0;
}

static int __init spinlock_test_init(void)
{
    int i, ret;
    ktime_t start, end;
    s64 elapsed_ns;
    unsigned long total_ops;
    unsigned long long throughput;

    if (nr_threads <= 0 || loops_per_thread == 0) {
        pr_err("Invalid parameters: threads=%d, loops=%lu\n",
               nr_threads, loops_per_thread);
        return -EINVAL;
    }

    pr_info("Starting spinlock test: %d threads, %lu loops each\n",
            nr_threads, loops_per_thread);

    threads = kmalloc_array(nr_threads, sizeof(struct task_struct *), GFP_KERNEL);
    if (!threads) {
        pr_err("Failed to allocate thread array\n");
        return -ENOMEM;
    }

    spin_lock_init(&test_lock);
    shared_counter = 0;

    start = ktime_get();

    for (i = 0; i < nr_threads; i++) {
        threads[i] = kthread_run(spinlock_thread_fn, NULL, "spinlock_test_%d", i);
        if (IS_ERR(threads[i])) {
            ret = PTR_ERR(threads[i]);
            pr_err("Failed to create thread %d: %d\n", i, ret);
            threads[i] = NULL;
            break;
        }
    }
    thread_count = i;

    for (i = 0; i < thread_count; i++) {
        if (threads[i])
            kthread_stop(threads[i]);
    }

    end = ktime_get();
    elapsed_ns = ktime_to_ns(ktime_sub(end, start));

    total_ops = shared_counter;
    /* 避免除零，且用 ULL 防止溢出 */
    if (elapsed_ns > 0) {
        throughput = (unsigned long long)(total_ops * 1000000000ULL) / elapsed_ns;
    } else {
        throughput = 0;
    }

    pr_info("Test completed:\n");
    pr_info("  Threads:          %d\n", thread_count);
    pr_info("  Loops per thread: %lu\n", loops_per_thread);
    pr_info("  Total operations: %lu\n", total_ops);
    pr_info("  Elapsed time:     %lld ns (%lld us)\n", elapsed_ns, elapsed_ns / 1000);
    pr_info("  Throughput:       %llu ops/sec\n", throughput);
    if (total_ops > 0)
        pr_info("  Avg lock overhead: %lld ns/op\n", elapsed_ns / total_ops);

    kfree(threads);
    threads = NULL;

    return (thread_count == nr_threads) ? 0 : -EIO;
}

static void __exit spinlock_test_exit(void)
{
    int i;
    if (threads) {
        for (i = 0; i < thread_count; i++) {
            if (threads[i]) {
                kthread_stop(threads[i]);
                threads[i] = NULL;
            }
        }
        kfree(threads);
        threads = NULL;
    }
    pr_info("Spinlock test module unloaded\n");
}

module_init(spinlock_test_init);
module_exit(spinlock_test_exit);
