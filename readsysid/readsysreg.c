#include <linux/module.h>
#include <linux/init.h>
#include <asm/cputype.h>

static int __init my_init(void)
{
    int u64 a;
    a = read_cpuid(ID_AA64MMFR1_EL1);
    pr_info("the value is %lu \n", a);
    return 0;
}

static void __exit my_exit(void)
{

    pr_info("inter the exit\n");
    return;
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
