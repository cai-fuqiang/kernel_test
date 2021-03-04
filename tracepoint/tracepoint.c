#include <linux/module.h>

#define CREATE_TRACE_POINTS 1
#include <trace/events/test.h>
struct delayed_work test_delayed_work;
void test_work_fn(struct work_struct *work)
{
    int i = 0;
    int j = 0;
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            trace_test_tracepoint(i, j);
        }
    }
}
static int __init my_init(void)
{
    INIT_DELAYED_WORK(&test_delayed_work, test_work_fn);
    schedule_delayed_work(&test_delayed_work, 10000);
    return 0;
}
static void __exit my_exit(void)
{
    trace_test_tracepoint(1, 2);
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
