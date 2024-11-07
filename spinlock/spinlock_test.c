#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/smp.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

static struct workqueue_struct *my_wq;
static struct delayed_work work1;
static struct delayed_work work2;
static spinlock_t my_lock; 

static void work_handler1(struct work_struct *work) {
    static unsigned long i = 0;
    if (!(i % 100)) {
       pr_info("Work handler on CPU1: %d\n", smp_processor_id());
    }
    spin_lock(&my_lock);
    mdelay(1000);
    spin_unlock(&my_lock);
    queue_delayed_work_on(1, my_wq, &work1, msecs_to_jiffies(10));
}

static void work_handler2(struct work_struct *work) {
    static unsigned long i = 0;
    if (!(i % 100)) {
        pr_info("Work handler on CPU2: %d\n", smp_processor_id());
    }
    spin_lock(&my_lock);
    mdelay(1000);
    spin_unlock(&my_lock);
    queue_delayed_work_on(2, my_wq, &work2, msecs_to_jiffies(10));
}

static void start_work_on_cpus(void) {
    // 在 CPU1 上排队
    queue_delayed_work_on(1, my_wq, &work1, msecs_to_jiffies(1000));

    // 在 CPU2 上排队
    queue_delayed_work_on(2, my_wq, &work2, msecs_to_jiffies(1000));
}

static int __init my_module_init(void) {
    pr_info("Initializing module\n");

    spin_lock_init(&my_lock);

    my_wq = create_workqueue("my_workqueue");

    INIT_DELAYED_WORK(&work1, work_handler1);
    INIT_DELAYED_WORK(&work2, work_handler2);

    start_work_on_cpus();

    return 0;
}

static void __exit my_module_exit(void) {
    pr_info("Exiting module\n");

    cancel_delayed_work_sync(&work1);
    cancel_delayed_work_sync(&work2);
    destroy_workqueue(my_wq);
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A kernel module to run work on CPU1 and CPU2 periodically");
