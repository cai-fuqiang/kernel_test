#include <linux/module.h>
#include <linux/init.h>

#include <asm/msr-index.h>
#include <asm/msr.h>

static unsigned long my_rdmsr(unsigned int msr)
{
        DECLARE_ARGS(val, low, high);                                                    
                                                                                         
        asm volatile("1: rdmsr\n"                                                        
                     "2:\n"                                                              
                     : EAX_EDX_RET(val, low, high) : "c" (msr));                         
                                                                                         
        return EAX_EDX_VAL(val, low, high);                                              
}

static int __init my_init(void)
{
#if 0
        u32 low, high = 0;

        pr_info("modules BEG\n");
        rdmsr(MSR_IA32_TSC_ADJUST, low, high);
        pr_info("IA32_TSC_ADJUST low(%x) high(%x)\n", low, high);

        pr_info("CET ==== \n");
        rdmsr(MSR_IA32_U_CET, low, high);
        pr_info("IA32_U_CET low(%x) high(%x)\n", low, high);
        rdmsr(MSR_IA32_S_CET, low, high);
        pr_info("IA32_S_CET low(%x) high(%x)\n", low, high);
#else
        unsigned long val;
        val = my_rdmsr(MSR_IA32_U_CET);

        pr_info("IA32_U_CET val (%lx) \n", val);
#endif

        return 0;
}
static void __exit my_exit(void)
{
        return;
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
