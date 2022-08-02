/*
 * NOTE: This example is works on x86 and powerpc.
 * Here's a sample kernel module showing the use of kprobes to dump a
 * stack trace and selected registers when _do_fork() is called.
 *
 * For more information on theory of operation of kprobes, see
 * Documentation/kprobes.txt
 *
 * You will see the trace data in /var/log/messages and on the console
 * whenever _do_fork() is invoked to create a new process.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

#include <linux/proc_fs.h>
#include <linux/jiffies.h>

#define NODE "wangfuqiang/call_count"
static struct proc_dir_entry *my_proc;
static struct proc_dir_entry *my_root;
#define KS 3213
static char kstring[KS];        /* should be less sloppy about overflows :) */
unsigned long _call_count = 0;
unsigned long loop_count = 0;
//unsigned long count_88 = 0;
//unsigned long count_88 = 0;


static ssize_t my_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
       int nbytes = sprintf(kstring, "%lu\n", _call_count);
       return simple_read_from_buffer(buf, lbuf, ppos, kstring, nbytes);
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t lbuf,
			loff_t *ppos)
{
        ssize_t rc;
        rc = simple_write_to_buffer(kstring, lbuf, ppos, buf, lbuf);
        sscanf(kstring, "%lu", &_call_count);
        pr_info("param has been set to %lu\n", _call_count);
        return rc;
}

static const struct file_operations my_proc_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write
};

int proctest_init(void)
{
    pr_info("inter the init\n");
    my_root = proc_mkdir("wangfuqiang", NULL);
    if (IS_ERR(my_root)) {
        pr_err("failed create the proc root dir\n");
        return -1;
    }
    my_proc = proc_create(NODE, 0, NULL, &my_proc_fops);

    if (IS_ERR(my_proc)) {
        pr_err("failed to create proc file %s\n", NODE);
        return -1;
    }
    return 0;
}
void proctest_exit(void)
{
    proc_remove(my_proc);
    pr_info("inter the exit\n");
    return;
}

//#define TEST 1
#define KPROBE_KEXEC 1

//static unsigned long get_timestamp(void) {
//	return ktime_get_ns();
//}

#ifdef KPROBE_KEXEC 
static int is_machine_kexec = 0;

//static unsigned long time_zone_88 = 0;
//static unsigned long time_zone_88_prev = 0;
//static unsigned long time_zone_88_next = 0;

static int handler_pre_ext_machine_kexec_0(struct kprobe *p, struct pt_regs *regs)
{
	is_machine_kexec = 1;
	return 0;
}

static int handler_post_ext_machine_kexec_0(struct kprobe *p, struct pt_regs *regs,
						unsigned long flags)
{
	printk("the is_machine_kexec is %d\n", is_machine_kexec);
	//is_machine_kexec = 0;
	return 0;
}

static int handler_fault_ext_machine_kexec_0(struct kprobe *p, struct pt_regs *regs,
						int  trapnr)
{
	return 0;
}

//static int handler_pre_ext___flush_cache_user_range_88(struct kprobe *p, struct pt_regs *regs)
//{
//	time_zone_88 = get_timestamp();
//	count_88++;
//	if (count_88 >= 10) {
//		printk("the time_zone_88(%ld) \n" ,
//				time_zone_88);
//		count_88 = 0;
//	}
//	return -1;
//}
//static int handler_pre_ext___flush_cache_user_range_88(struct kprobe *p, struct pt_regs *regs)
//{
//
//	time_zone_88_next = get_timestamp();
//	count_88++;
//	if (count_88 >= 10) {
//		printk("time_zone_88_prev(%ld) \n"
//			"time_zone_88_next(%ld) \n"
//				"88 - 88 (%ld)\n",
//				time_zone_88_prev, time_zone_88_next,
//				time_zone_88_next - time_zone_88_prev);
//		count_88 = 0;
//	}
//	time_zone_88_prev = time_zone_88_next;
//	return -1;
//}
//
//static int  handler_post_ext___flush_cache_user_range_88(struct kprobe *p, struct pt_regs *regs,
//						unsigned long flags)
//{
//
//	return -1;
//}
//static int  handler_post_ext___flush_cache_user_range_88(struct kprobe *p, struct pt_regs *regs,
//						unsigned long flags)
//{
//
//	return -1;
//}
//static int handler_fault_ext___flush_cache_user_range_88(struct kprobe *p, struct pt_regs *regs,
//						int  trapnr)
//{
//	return -1;	
//}
//static int handler_fault_ext___flush_cache_user_range_88(struct kprobe *p, struct pt_regs *regs,
//						int  trapnr)
//{
//	return -1;	
//}
#endif

#include "handler_ext_condition_module.h"
#include "handler_ext_none_module.h"

#ifdef KPROBE_KEXEC 
DEF_FUNC_EXT_CONDITION_HANDLE(__flush_dcache_area_for_kprobe, is_machine_kexec)
DEF_OBJ_EXT_CONDITION_HANDLE(__flush_dcache_area_for_kprobe, is_machine_kexec, 32) 	//dc

DEF_FUNC_EXT_CONDITION_HANDLE(__flush_cache_user_range_for_kprobe, is_machine_kexec)
//DEF_OBJ_EXT_NONE_HANDLE(__flush_cache_user_range_for_kprobe, 32) 		//dc
//DEF_OBJ_EXT_NONE_HANDLE(__flush_cache_user_range_for_kprobe, 40) 		//dc
//DEF_OBJ_EXT_NONE_HANDLE(__flush_cache_user_range_for_kprobe, 88) 		//ic
//DEF_OBJ_EXT_NONE_HANDLE(__flush_cache_user_range_for_kprobe, 112) 		//ic

DEF_OBJ_EXT_CONDITION_HANDLE(__flush_cache_user_range_for_kprobe, is_machine_kexec, 32) 		//dc
DEF_OBJ_EXT_CONDITION_HANDLE(__flush_cache_user_range_for_kprobe, is_machine_kexec, 40) 		//dc
DEF_OBJ_EXT_CONDITION_HANDLE(__flush_cache_user_range_for_kprobe, is_machine_kexec, 88) 		//ic
DEF_OBJ_EXT_CONDITION_HANDLE(__flush_cache_user_range_for_kprobe, is_machine_kexec, 112) 		//ic
DEF_OBJ_EXT_CONDITION_HANDLE(__flush_cache_user_range_for_kprobe, is_machine_kexec, 132) 		//ic ret before
DEF_OBJ_EXT_CONDITION_HANDLE(__flush_cache_user_range_for_kprobe, is_machine_kexec, 136) 		//ic fixup
DEF_OBJ_EXT_CONDITION_HANDLE(__flush_cache_user_range_for_kprobe, is_machine_kexec, 140) 		//ic ret

#endif

#ifdef TEST
DEF_FUNC_EXT_NONE_HANDLE(kvm_vm_ioctl)
DEF_OBJ_EXT_NONE_HANDLE(kvm_vm_ioctl, 12)
#endif

#include "handler_module.h"

#ifdef KPROBE_KEXEC 
DEF_KP_FUNC(machine_kexec)
DEF_KP_FUNC(__flush_dcache_area_for_kprobe)
DEF_KP_OBJ(__flush_dcache_area_for_kprobe, 32)
DEF_KP_FUNC(__flush_cache_user_range_for_kprobe)
DEF_KP_OBJ(__flush_cache_user_range_for_kprobe, 32)
DEF_KP_OBJ(__flush_cache_user_range_for_kprobe, 40)
DEF_KP_OBJ(__flush_cache_user_range_for_kprobe, 88)
DEF_KP_OBJ(__flush_cache_user_range_for_kprobe, 112)
DEF_KP_OBJ(__flush_cache_user_range_for_kprobe, 132)
DEF_KP_OBJ(__flush_cache_user_range_for_kprobe, 136)
DEF_KP_OBJ(__flush_cache_user_range_for_kprobe, 140)

//DEF_KP_OBJ(vectors, 512)
#endif

#ifdef TEST
DEF_KP_FUNC(kvm_vm_ioctl)
DEF_KP_OBJ(kvm_vm_ioctl, 12)
#endif

extern int proctest_init(void);
extern void proctest_exit(void);
static int __init kprobe_init(void)
{
#ifdef KPROBE_KEXEC 
	register_kprobe_func(machine_kexec);
	register_kprobe_func(__flush_dcache_area_for_kprobe);
	register_kprobe_obj(__flush_dcache_area_for_kprobe, 32);
	register_kprobe_func(__flush_cache_user_range_for_kprobe);
	register_kprobe_obj(__flush_cache_user_range_for_kprobe, 32);
	register_kprobe_obj(__flush_cache_user_range_for_kprobe, 40);
	register_kprobe_obj(__flush_cache_user_range_for_kprobe, 88);
	register_kprobe_obj(__flush_cache_user_range_for_kprobe, 112);
	register_kprobe_obj(__flush_cache_user_range_for_kprobe, 132);
	register_kprobe_obj(__flush_cache_user_range_for_kprobe, 136);
	register_kprobe_obj(__flush_cache_user_range_for_kprobe, 140);
#endif

#ifdef TEST
	register_kprobe_func(kvm_vm_ioctl);
	register_kprobe_obj(kvm_vm_ioctl, 12);
#endif
	proctest_init();
	return 0;

//ERR_REG_OBJ(vectors, 512):
//	unregister_kprobe_obj(vectors, 512);
ERR_REG_OBJ(__flush_cache_user_range_for_kprobe, 140):
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 140);
ERR_REG_OBJ(__flush_cache_user_range_for_kprobe, 136):
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 136);
ERR_REG_OBJ(__flush_cache_user_range_for_kprobe, 132):
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 132);
ERR_REG_OBJ(__flush_cache_user_range_for_kprobe, 112):
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 112);
ERR_REG_OBJ(__flush_cache_user_range_for_kprobe, 88):
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 88);
ERR_REG_OBJ(__flush_cache_user_range_for_kprobe, 40):
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 40);
ERR_REG_OBJ(__flush_cache_user_range_for_kprobe, 32):
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 32);
ERR_REG_FUNC(__flush_cache_user_range_for_kprobe):
	unregister_kprobe_func(__flush_cache_user_range_for_kprobe);
ERR_REG_OBJ(__flush_dcache_area_for_kprobe, 32):
	unregister_kprobe_obj(__flush_dcache_area_for_kprobe, 32);
ERR_REG_FUNC(__flush_dcache_area_for_kprobe):
	unregister_kprobe_func(__flush_dcache_area_for_kprobe);
ERR_REG_FUNC(machine_kexec):
	unregister_kprobe_func(machine_kexec);
	return -1;
}

static void __exit kprobe_exit(void)
{
#ifdef KPROBE_KEXEC 
	unregister_kprobe_func(machine_kexec);
	unregister_kprobe_func(__flush_dcache_area_for_kprobe);
	unregister_kprobe_obj(__flush_dcache_area_for_kprobe, 32);
	unregister_kprobe_func(__flush_cache_user_range_for_kprobe);
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 32);
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 40);
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 88);
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 112);
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 132);
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 136);
	unregister_kprobe_obj(__flush_cache_user_range_for_kprobe, 140);
	//unregister_kprobe_obj(vectors, 512);
#endif

#ifdef TEST
	unregister_kprobe_func(kvm_vm_ioctl);
	unregister_kprobe_obj(kvm_vm_ioctl, 12);
#endif
	proctest_exit();
	return;
}

module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");
