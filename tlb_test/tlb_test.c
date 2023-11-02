#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/mm_types.h>
#include <linux/gfp.h>
//#include <linux/page_table_check.h>
//#include <asm-generic/page.h>
//#include <linux/pgtable.h>
////#include <asm/pgtable.h>
///
#include <linux/mm.h>
#include <asm/tlbflush.h>

static struct proc_dir_entry *my_page_proc = NULL;
static struct proc_dir_entry *my_page_flags_proc = NULL;

static struct proc_dir_entry *my_page_misc_proc = NULL;

static struct proc_dir_entry *my_root = NULL;
//static char kstring[32];        /* should be less sloppy about overflows :) */

unsigned long g_page_val = 0;

//struct page * g_page = NULL;

//unsigned long g_user_addr = 0;

struct alloc_page_info {
        struct page *page;
        unsigned long virt;
        pgd_t *pgd;
        p4d_t *p4d;
        pud_t *pud;
        pmd_t *pmd;
        pte_t *pte;
        pte_t org_pte;
};


struct alloc_page_info g_page_info = {
        .page = NULL,
        .virt = 0,
        .pgd = NULL, 
        .p4d = NULL, 
        .pud = NULL, 
        .pmd = NULL, 
        .pte = NULL
};

pte_t *my_pte_offset_map(pmd_t *pmd, unsigned long addr)
{
        pmd_t pmlval;

        pmlval = *pmd;
        return (pte_t *)pmd_page_vaddr(pmlval) + pte_index(addr);
}

static void init_page_info(struct page *page)
{
        g_page_info.page = page;
        g_page_info.virt = (unsigned long )page_to_virt(g_page_info.page);
        g_page_info.pgd = pgd_offset(current->mm,               g_page_info.virt);
        g_page_info.p4d = p4d_offset(g_page_info.pgd,           g_page_info.virt);
        g_page_info.pud = pud_offset(g_page_info.p4d,           g_page_info.virt);
        g_page_info.pmd = pmd_offset(g_page_info.pud,           g_page_info.virt);

        g_page_info.pmd = pud_large(*g_page_info.pud) ? (pmd_t *)g_page_info.pud :
                          pmd_offset(g_page_info.pud, g_page_info.virt);
                        
        g_page_info.pte = pmd_large(*g_page_info.pmd) ? (pte_t *)g_page_info.pmd :
                        my_pte_offset_map(g_page_info.pmd,    g_page_info.virt);

        g_page_info.org_pte = *g_page_info.pte;
        return ;
}

#define get_vaild_ptr_val(ptr)  ((ptr) ? (*(unsigned long *)ptr) : 0)

static void print_page_info(void)
{
        if (!g_page_info.page) {
                return;
        }

        pr_info("the page (%lx) : \n", (unsigned long)g_page_info.page);

        pr_info("virt (%lx) \n"
                "pgd : %lx ==> %lx\n"
                "p4d : %lx ==> %lx\n"
                "pud : %lx ==> %lx\n"
                "pmd : %lx ==> %lx\n"
                "pte : %lx ==> %lx\n",
                g_page_info.virt,
                (unsigned long)g_page_info.pgd, get_vaild_ptr_val(g_page_info.pgd),
                (unsigned long)g_page_info.p4d, get_vaild_ptr_val(g_page_info.p4d),
                (unsigned long)g_page_info.pud, get_vaild_ptr_val(g_page_info.pud),
                (unsigned long)g_page_info.pmd, get_vaild_ptr_val(g_page_info.pmd),
                (unsigned long)g_page_info.pte, get_vaild_ptr_val(g_page_info.pte));
}

static void clean_page_info(void)
{
        memset(&g_page_info, 0, sizeof(g_page_info));
}
#if 0
static ssize_t my_page_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
        int nbytes = sprintf(kstring, "%lx\n", (unsigned long)g_page_info.page);
       
        pr_info("enter func (%s) current_mm (%lx)\n", __FUNCTION__, 
                        (unsigned long )current->mm);
        print_page_info();
        return simple_read_from_buffer(buf, lbuf, ppos, kstring, nbytes);
}
#endif
static ssize_t my_page_write(struct file *file, const char __user *buf, size_t lbuf,
			loff_t *ppos)
{
        int ret;
        unsigned long val;
        struct page *g_page = g_page_info.page;
        ret = kstrtoul_from_user(buf, lbuf, 0, &val);

        if (ret)
                return ret;
        pr_info("enter func (%s) current_mm (%lx)\n", __FUNCTION__, 
                        (unsigned long )current->mm);
        //g_user_addr = val;
        if (!g_page && val > 0) {
                g_page = alloc_pages(GFP_KERNEL, 0);
                if (g_page) {
                        pr_info("the new g_page addr is %lx\n", (unsigned long)g_page);
                        init_page_info(g_page);
                        print_page_info();
                }
        } else if (g_page && val == 0) {
                __free_pages(g_page, 0);
                pr_info("free page %lx\n", (unsigned long)g_page);
                clean_page_info();
        }
        return lbuf;
}

#define SET_PAGE_FLAGS(pte, cond, flag)  \
        (                                \
         (pte) = cond ? (pte) | (flag) : \
                (pte) & (~(flag))        \
        )

void my_test_all(bool dirty, bool present, bool writable)
{
        pte_t pte;
        pte_t org_pte;
        //pte_t tmp_pte;

        pr_info("=========test all =========BEG[%s][%s][%s]\n", 
                        dirty ? "dirty":"", present ? "present":"",
                        writable ? "writable":"");


        pte.pte = get_vaild_ptr_val(g_page_info.pte);
        org_pte = g_page_info.org_pte;

        pr_info("pte (%lx), org_pte (%lx)\n", pte.pte, org_pte.pte);

        /* 
         * some operation can't cause fixup PF, The next time 
         * you enter this function, we need to fix invalid PTE.
         */
        if (pte.pte != org_pte.pte) {
                set_pte(g_page_info.pte, org_pte);
                __flush_tlb_all();
                pte.pte = org_pte.pte;
        }

        //1. set && clean dirty bit
	pr_info("=====set dirty\n");
        pr_info("new old pte is %lx \n", pte.pte);
        SET_PAGE_FLAGS(pte.pte, dirty, _PAGE_DIRTY);
        pr_info("new pte is %lx ", pte.pte);
        set_pte(g_page_info.pte, pte);

	pr_info("=====flush tlb \n");
        //2. flush tlb
        __flush_tlb_all();
	pr_info("===== set present && rw  \n");
        //3. modify present writable bit local
        pr_info("old pte is %lx \n", pte.pte);
        SET_PAGE_FLAGS(pte.pte, present, _PAGE_PRESENT);
        SET_PAGE_FLAGS(pte.pte, writable, _PAGE_RW);
        pr_info("new old pte is %lx \n", pte.pte);

	pr_info("=====read page establish tlb\n");
        
        //4. read address  establish tlb
        //use global val, avoid gcc optimize
        g_page_val = *(unsigned long *)g_page_info.virt;
        //5. set pte
        set_pte(g_page_info.pte, pte);
        //6. write this page 
        //
        *(unsigned long *)g_page_info.virt = 0x0;

	//pr_info("=====not flush tlb \n");
	//pr_info("=====write this page \n");

        //6. set org pte
        set_pte(g_page_info.pte, org_pte);
        pr_info("=========test all =========END\n");
}

static ssize_t my_page_flags_write(struct file *file, const char __user *buf, size_t lbuf,
			loff_t *ppos)
{
        int ret = 0;
        unsigned long val;
        struct page *g_page = g_page_info.page;
        pte_t pte;
        pr_info("enter func (%s) current_mm (%lx)\n", __FUNCTION__, 
                        (unsigned long )current->mm);
        if (!g_page) {
                goto end;
        }
        ret = kstrtoul_from_user(buf, lbuf, 0, &val);
        if (val < 0) {
                goto end;
        }
        pte.pte = get_vaild_ptr_val(g_page_info.pte);
        if (!pte.pte) {
                goto end;
        }
        
        pr_info("val is %lu\n",val);
        pr_info("old pte is %lx\n", pte.pte);

        pte.pte = val > 0 ? pte.pte & (~_PAGE_DIRTY) : pte.pte | _PAGE_DIRTY;
        //pr_info("new pte is %lx DEG: %lx\n", pte.pte, pte.pte & (~_PAGE_DIRTY) );
        pr_info("new pte is %lx \n", pte.pte);
        set_pte(g_page_info.pte, pte);
end:
        return lbuf;
}

static ssize_t my_page_misc_write(struct file *file, const char __user *buf, size_t lbuf,
			loff_t *ppos)
{
        int ret = 0;
        unsigned long val;
        unsigned long page_val;

        if (!g_page_info.page) {
                goto end;
        }
        ret = kstrtoul_from_user(buf, lbuf, 0, &val);
        if (val < 0) {
                goto end;
        }

        if (val == 1) {
                pr_info("write this page \n");
                *(unsigned long *)g_page_info.virt = 0xabcdef;
        } else if (val == 0) {
                page_val = *(unsigned long *)g_page_info.virt;
                pr_info("read this page val is %lx\n", page_val);
        } else if (val == 2) {
                //dirty, not present, rw
                my_test_all(true, false, true);
        } else if (val == 3) {
                //not dirty, not present, rw
                my_test_all(false, false, true);
        } else if (val == 4) {
                //dirty, present, read only
                my_test_all(true, true, false);
        } else if (val == 5) {
                //not dirty, present, read only
                my_test_all(false, true, false);
        } else if (val == 6) {
                //not dirty, present, rw
                my_test_all(false, true, true);
        }
end:
        return lbuf;

}
#if 0
//static const struct file_operations my_page_proc_fops = {
static const  struct proc_ops my_page_proc_fops = {
        //.proc_read = my_page_read,
        .proc_write = my_page_write
};
static const  struct proc_ops my_page_flags_proc_fops = {
        .proc_write = my_page_flags_write
};

static const struct proc_ops my_page_misc_proc_fops = {
        .proc_write = my_page_misc_write
};
#endif
#define FEDORA 

#ifdef FEDORA
#define PROC_FOPS_ONLY_WRITE(_name)             \
        static const struct proc_ops _name ## _proc_fops = {    \
                .proc_write = _name ##_write                           \
        };
#else
#define PROC_FOPS_ONLY_WRITE(_name)             \
        static const struct file_operations _name ## _proc_fops = {     \
                .owner = THIS_MODULE,                   \
                .write = _name ##_write,                        \
        };
#endif

PROC_FOPS_ONLY_WRITE(my_page)
PROC_FOPS_ONLY_WRITE(my_page_flags)
PROC_FOPS_ONLY_WRITE(my_page_misc)

void free_proc_file(void)
{
        if (my_page_proc) {
                proc_remove(my_page_proc);
                my_page_proc = NULL;
        }
        if (my_page_flags_proc) {
                proc_remove(my_page_flags_proc);
                my_page_flags_proc = NULL;
        }

        if (my_page_misc_proc) {
                proc_remove(my_page_misc_proc);
                my_page_misc_proc = NULL;
        }
        if (my_root) { 
                proc_remove(my_root);
                my_root = NULL; 
        }
        return ;
}

#define create_proc_file(_name) \
        do {                                                                                    \
                _name = proc_create("fuqiang/"#_name, 0, NULL, & _name ## _fops);                 \
                if (IS_ERR(_name)) {                                                            \
                        pr_err("failed create the proc file %s\n", #_name);                     \
                        goto err;                                                               \
                }                                                                               \
        } while(0)

static int __init my_init(void)
{
        pr_info("inter the init\n");
        my_root = proc_mkdir("fuqiang", NULL);
        if (IS_ERR(my_root)) {
                pr_err("failed create the proc root dir\n");
                goto err;
        }

        create_proc_file(my_page_proc);
        create_proc_file(my_page_flags_proc);
        create_proc_file(my_page_misc_proc);
end:
        return 0;
err:
        free_proc_file();
        goto end;
}
static void __exit my_exit(void)
{
        pr_info("inter the exit\n");
        free_proc_file();

        if (g_page_info.page) {
                __free_pages(g_page_info.page, 0);
        }
        return;
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
