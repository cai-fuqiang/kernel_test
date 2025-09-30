#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/smp.h>
#include <linux/init.h>
#include <linux/param.h>
#include <asm/delay.h>

static int target_cpu = 0;
module_param(target_cpu, int, 0444);
MODULE_PARM_DESC(target_cpu, "CPU to run hrtimer on");

static unsigned long period_ms = 1000;
module_param(period_ms, ulong, 0444);
MODULE_PARM_DESC(period_ms, "Timer period in ms");

static struct hrtimer my_timer;

static enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
    ktime_t now = ktime_get();
    ktime_t delta;

    static int i = 0;

    delta = ktime_sub_ns(now, timer->node.expires);

    pr_info("hrtimer fired on cpu %d, now(%llx) delta(%llx)\n", smp_processor_id(), now, delta );

    // add a delay
    udelay(210);

    hrtimer_add_expires_ns(timer, ms_to_ktime(period_ms));
    return HRTIMER_RESTART;
}

static void start_timer_on_cpu(void *info)
{
    ktime_t period = ms_to_ktime(period_ms);
    ktime_t now = ktime_get();
    ktime_t expire;

    expire = ktime_add_ns(now, period);

    pr_info("Starting hrtimer on cpu %d\n", smp_processor_id());
    hrtimer_setup(&my_timer,  timer_callback, CLOCK_MONOTONIC,
		   HRTIMER_MODE_ABS_PINNED_HARD);

    hrtimer_start(&my_timer, expire, HRTIMER_MODE_ABS_PINNED_HARD);
}

static int __init mymodule_init(void)
{
    if (target_cpu < 0 || target_cpu >= num_online_cpus()) {
        pr_err("Invalid target_cpu %d\n", target_cpu);
        return -EINVAL;
    }

    pr_info("Module loaded. Starting timer on CPU %d every %lums\n", target_cpu, period_ms);

    smp_call_function_single(target_cpu, start_timer_on_cpu, NULL, 1);

    return 0;
}

static void __exit mymodule_exit(void)
{
    pr_info("Module exit. Cancelling timer.\n");

    hrtimer_cancel(&my_timer);
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenAI ChatGPT");
MODULE_DESCRIPTION("Periodic hrtimer on specific CPU");
