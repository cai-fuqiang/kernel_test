#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <asm/tlbflush.h>
#include <linux/mm.h>
#include <linux/kallsyms.h>

#define NODE "benshushu/my_proc"
static struct proc_dir_entry *my_proc;
static struct proc_dir_entry *my_root;

unsigned char modify_func_dst_code[] = 
{
	0xf, 0x1, 0xc1    //intel
};

unsigned char modify_func_src_code[] = 
{
	0xf, 0x1, 0xd9
};

#define MODIFY_FUNC_NAME "kvm_kick_cpu"
#define MODIFY_FUNC_OFFSET  (39)
#define MODIFY_FUNC_CODE_LEN (sizeof(modify_func_src_code))

static bool debug = 1;
#define KS	1024
static char kstring[KS];        /* should be less sloppy about overflows :) */

static unsigned long modify_func_ptr = 0;

#define MODIFY_CODE_PTR (modify_func_ptr + MODIFY_FUNC_OFFSET)

#define my_pr_info(...) 				\
	do {						\
		if (debug)				\
			pr_info( __VA_ARGS__);	\
	} while(0)
struct page_info {
        unsigned long virt;
        pgd_t *pgd;
        pud_t *pud;
        pmd_t *pmd;
        pte_t *pte;
        pte_t org_pte;
};

pte_t *my_pte_offset_map(pmd_t *pmd, unsigned long addr)
{
        pmd_t pmlval;

        pmlval = *pmd;
        return (pte_t *)pmd_page_vaddr(pmlval) + pte_index(addr);
}
static void init_page_info(unsigned long virt, struct page_info *pg_info)
{
        pg_info->virt = virt;
        pg_info->pgd = pgd_offset(current->mm,            pg_info->virt);
        pg_info->pud = pud_offset(pg_info->pgd,           pg_info->virt);
        pg_info->pmd = pmd_offset(pg_info->pud,           pg_info->virt);

        pg_info->pmd = pud_large(*pg_info->pud) ? (pmd_t *)pg_info->pud :
                          pmd_offset(pg_info->pud, pg_info->virt);
                        
        pg_info->pte = pmd_large(*pg_info->pmd) ? (pte_t *)pg_info->pmd :
                        my_pte_offset_map(pg_info->pmd,    pg_info->virt);

        pg_info->org_pte = *pg_info->pte;

        return ;
}
#define get_vaild_ptr_val(ptr)  ((ptr) ? (*(unsigned long *)ptr) : 0)

static void print_page_info(struct page_info *pg_info)
{
        my_pr_info("virt (%lx) \n"
                "pgd : %lx ==> %lx\n"
                "pud : %lx ==> %lx\n"
                "pmd : %lx ==> %lx\n"
                "pte : %lx ==> %lx\n",
                pg_info->virt,
                (unsigned long )pg_info->pgd, get_vaild_ptr_val(pg_info->pgd),
                (unsigned long )pg_info->pud, get_vaild_ptr_val(pg_info->pud),
                (unsigned long )pg_info->pmd, get_vaild_ptr_val(pg_info->pmd),
                (unsigned long )pg_info->pte, get_vaild_ptr_val(pg_info->pte));
}

#define SET_PAGE_FLAGS(pte, cond, flag)  \
        (                                \
         (pte) = cond ? (pte) | (flag) : \
                (pte) & (~(flag))        \
        )

static void modify_pgtable_flags(struct page_info *pg_info, unsigned long flags, bool enable)
{
	pte_t pte;

	pte.pte = get_vaild_ptr_val(pg_info->pte);
	
	SET_PAGE_FLAGS(pte.pte, enable, flags);

	set_pte(pg_info->pte, pte);
}

static void revert_pgtable_flags(struct page_info *pg_info)
{
	set_pte(pg_info->pte, pg_info->org_pte);
}

static struct page_info g_page_info = {
	.virt = 0
};

static void modify_pagetlb_by_ptr(unsigned long ptr)
{
	if (!g_page_info.virt) {
		init_page_info(ptr, &g_page_info);
	}

	my_pr_info("modify page table prev, print \n");
	print_page_info(&g_page_info);

	modify_pgtable_flags(&g_page_info, _PAGE_RW, 1);
	my_pr_info("modify page table post, print \n");
	print_page_info(&g_page_info);
	/* 
	 * avoid org tlb has not RW bit, may cause kernel panic
	 *
	 * see
	 * https://github.com/cai-fuqiang/kernel_test/tree/master/tlb_test
	 */
	__flush_tlb_all();


}

static void revert_pgtable(void)
{
	my_pr_info("revert page table post, print \n");
	print_page_info(&g_page_info);
	revert_pgtable_flags(&g_page_info);
	__flush_tlb_all();
	print_page_info(&g_page_info);
	my_pr_info("revert page table post, print END \n");
}

static void modify_code(char *real_dst, char *dst, char *src, size_t len)
{
	if (memcmp(real_dst, dst, len))
	{
		my_pr_info("the src code and real code is not equal, skip\n");
		return;
	}
	memcpy(real_dst, src, len);

}

static void execute_vmmcall(void)
{
	__asm__("vmmcall");
	return;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t lbuf,
			loff_t *ppos)
{
	size_t rc;
	int ops = 0;
        rc = simple_write_to_buffer(kstring, lbuf, ppos, buf, lbuf);
	sscanf(kstring, "%d", &ops);
	switch (ops) {
	case 0:
		my_pr_info("modify pgtable, function ptr (%lx)\n",(unsigned long)execute_vmmcall );
		modify_pagetlb_by_ptr((unsigned long)execute_vmmcall);
		break;
	case 1:
		my_pr_info("execute vmmcall");
		execute_vmmcall();
		break;
	case 2:
		my_pr_info("modify code %s:%d\n", MODIFY_FUNC_NAME, MODIFY_FUNC_OFFSET);
		modify_pagetlb_by_ptr(modify_func_ptr);
		my_pr_info("modify this code\n");
		modify_code((char *)MODIFY_CODE_PTR, (char *)modify_func_dst_code, (char *)modify_func_src_code, MODIFY_FUNC_CODE_LEN);
		revert_pgtable();
		break;
	case 3:
		my_pr_info("revert modify code %s:%d\n", MODIFY_FUNC_NAME, MODIFY_FUNC_OFFSET);
		modify_pagetlb_by_ptr(modify_func_ptr);
		my_pr_info("modify this code\n");
		modify_code((char *)MODIFY_CODE_PTR, (char *)modify_func_src_code, (char *)modify_func_dst_code, MODIFY_FUNC_CODE_LEN);
		revert_pgtable();
		break;
	default:
		break;

	}
	return rc;
}

static const struct file_operations my_proc_fops = {
    .owner = THIS_MODULE,
    .write = my_write
};

static  int find_modify_symbol(void)
{
	unsigned long function = 0;
	
	function = kallsyms_lookup_name(MODIFY_FUNC_NAME);

	if (function) {
		my_pr_info("get function(%s):(%lx)\n", MODIFY_FUNC_NAME, function);
		modify_func_ptr = function;
		return 0;
	} else {
		my_pr_info("not found function(%s)\n",  MODIFY_FUNC_NAME);
		return -1;
	}
	
}

static int __init my_init(void)
{
    int ret = 0;
    ret = find_modify_symbol();
    if (ret < 0) {
	goto end;
    }
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
end:
    return ret;
}
static void __exit my_exit(void)
{
    proc_remove(my_proc);
    pr_info("inter the exit\n");
    return;
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
