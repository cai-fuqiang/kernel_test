#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/smp.h>

#define NODE "kick_all_cpu_test/kick"
int count = 0;
static struct proc_dir_entry *my_proc;
static struct proc_dir_entry *my_root;
#define KS 3213
static char kstring[KS] = "";        /* should be less sloppy about overflows :) */


static ssize_t my_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
       int nbytes = sprintf(kstring, "%d\n", count);
       return simple_read_from_buffer(buf, lbuf, ppos, kstring, nbytes);
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t lbuf,
			loff_t *ppos)
{
        kick_all_cpus_sync();
        return lbuf;
}

static const struct file_operations my_proc_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write
};

static int __init my_init(void)
{
    pr_info("inter the init\n");
    my_root = proc_mkdir("kick_all_cpu_test", NULL);
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
static void __exit my_exit(void)
{
    pr_info("inter the exit\n");
    proc_remove(my_proc);
    proc_remove(my_root);
    return;
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
