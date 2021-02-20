#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>

#define NODE "benshushu/my_proc"
static int param = 100;
static struct proc_dir_entry *my_proc;
static struct proc_dir_entry *my_root;
#define KS 3213
static char kstring[KS];        /* should be less sloppy about overflows :) */


static ssize_t my_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
       int nbytes = sprintf(kstring, "%d\n", param);
       return simple_read_from_buffer(buf, lbuf, ppos, kstring, nbytes);
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t lbuf,
			loff_t *ppos)
{
        ssize_t rc;
        rc = simple_write_to_buffer(kstring, lbuf, ppos, buf, lbuf);
        sscanf(kstring, "%d", &param);
        pr_info("param has been set to %d\n", param);
        return rc;
}

static const struct file_operations my_proc_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write
};

static int __init my_init(void)
{
    pr_info("inter the init\n");
    my_root = proc_mkdir("benshushu", NULL);
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
    return;
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
