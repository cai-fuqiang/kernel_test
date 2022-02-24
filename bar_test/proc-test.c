#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <asm/pgtable.h>

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

int add_num(int a, int b)
{
	int i = 0;
	for (i = 0; i < 100; i++){
		a++;
		b++;
	}
	return a + b;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t lbuf,
			loff_t *ppos)
{
        ssize_t rc;
	int a = 100;
	int b = 200;
	add_num(a + b, a / b);
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

char my_buf[4096 * 4] = "";

static int __init my_init(void)
{
    struct mm_struct *mm = current->mm;
    unsigned long bufptr = &my_buf;
    unsigned long bufptr_align = bufptr >> PAGE_SHIFT << PAGE_SHIFT;
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long pte_data;
    unsigned long pte_data_org;
    unsigned long pte_low_flag;
    unsigned long pte_high_flag;
    unsigned long pte_high_align;
    unsigned int pte_high_int;
    unsigned int *pte_high_ptr;
    int i = 0;

    unsigned long *pte_data_ptr;
    pr_info("inter the init\n");
    printk("the my_buf ptr is %llx \n", my_buf);
    printk("the align my_buf ptr align is %lx \n", bufptr_align);
    printk("the pagesize is %u\n", PAGE_SIZE);
    printk("the mm is %llx\n, active mm is %llx\n", mm, current->active_mm);

    pgd = pgd_offset(mm, bufptr_align);
    p4d = p4d_offset(pgd, bufptr_align);
    pud = pud_offset(p4d, bufptr_align);
    pmd = pmd_offset(pud, bufptr_align);
    pte = pte_offset_kernel(pmd, bufptr_align);

    printk("pgd(%lx) p4d(%lx) pud(%lx) pmd(%lx) pte(%lx)\n",
           pgd->pgd, p4d->p4d, pud->pud, pmd->pmd, pte->pte);

    pte_data = pte->pte;
    pte_data_org = pte->pte;
    pte_low_flag = pte_data & ((1 << PAGE_SHIFT) - 1);
    //printk("page shift is %lx\n",PAGE_SHIFT);
    //printk("page align is %lx\n", (1 << PAGE_SHIFT) - 1);
    printk("pte_low_flag is %lx\n", pte_low_flag);

    pte_high_int = 1 << 16;
    pte_high_int = ~(pte_high_int - 1);
    pte_high_align = 0;
    pte_high_ptr = (unsigned int *)&pte_high_align + 1;

    *pte_high_ptr = pte_high_int;
    printk("the pte_high align is %lx\n", pte_high_align);
    pte_high_flag = pte_data & pte_high_align;
    printk("the pte_high flag is %lx\n", pte_high_flag);

    pte_data = pte_high_flag | pte_low_flag | 0xc0100000;
    printk("the pte_data is  %lx\n", pte_data);

    pte->pte = pte_data;

    printk("the pte_data is  %lx\n", pte_data);
    pte_data_ptr =  (unsigned long *)bufptr_align;
    printk("the pte_data_ptr is  %lx\n", pte_data_ptr);
    for (i = 0; i < 10; i++) {
        printk("the pte_data_ptr data is  %llx\n", *pte_data_ptr);
        pte_data_ptr++;
    }
    pte->pte = pte_data_org;
    //printk("the pte_data_ptr is %lx\n", *pte_data_ptr);
   // printk("pte_high_flag is %lx\n", pte_high_flag);
    //my_root = proc_mkdir("benshushu", NULL);
    //if (IS_ERR(my_root)) {
    //    pr_err("failed create the proc root dir\n");
    //    return -1;
    //}
    //my_proc = proc_create(NODE, 0, NULL, &my_proc_fops);

    //if (IS_ERR(my_proc)) {
    //    pr_err("failed to create proc file %s\n", NODE);
    //    return -1;
    //}
    return 0;
}
static void __exit my_exit(void)
{
    //proc_remove(my_proc);
    pr_info("inter the exit\n");
    return;
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
