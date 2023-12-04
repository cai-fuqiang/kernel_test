#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/mm_types.h>

#include <linux/mm.h>
#include <asm/tlbflush.h>
#include <asm/paravirt_types.h>
#include <linux/kallsyms.h>

extern struct pv_mmu_ops pv_mmu_ops;
static bool debug;

#define my_pr_info(...) 				\
	do {						\
		if (debug)				\
			pr_info( __VA_ARGS__);	\
	} while(0)
struct page_info {
        unsigned long virt;
        pgd_t *pgd;
        p4d_t *p4d;
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
        pg_info->p4d = p4d_offset(pg_info->pgd,           pg_info->virt);
        pg_info->pud = pud_offset(pg_info->p4d,           pg_info->virt);
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
                "p4d : %lx ==> %lx\n"
                "pud : %lx ==> %lx\n"
                "pmd : %lx ==> %lx\n"
                "pte : %lx ==> %lx\n",
                pg_info->virt,
                (unsigned long )pg_info->pgd, get_vaild_ptr_val(pg_info->pgd),
                (unsigned long )pg_info->p4d, get_vaild_ptr_val(pg_info->p4d),
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

static void *tlb_flush_func = NULL;
static void *symbol_native_tlb_flush = NULL;

static void modify_pv_tlb_ptr(void *ptr)
{
	if (!g_page_info.virt) {
		init_page_info((unsigned long )&pv_mmu_ops, &g_page_info);
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

	WRITE_ONCE(pv_mmu_ops.flush_tlb_others, ptr);

	revert_pgtable_flags(&g_page_info);
	__flush_tlb_all();
	my_pr_info("revert page table post, print \n");
	print_page_info(&g_page_info);

}

static inline bool is_native_tlb_flush(void)
{
	return tlb_flush_func == symbol_native_tlb_flush;
}

static inline bool skip_modify(void ) {
	if (!symbol_native_tlb_flush) {
		pr_info("native_flush_tlb_others() is not found SKIP !!!\n");
		goto skip;
	} else if (is_native_tlb_flush()){
		pr_info("the flush func is native flush, skip\n");
		goto skip;
	}
	return false;
skip:
	return true;

}

static int __init my_init(void)
{
	tlb_flush_func = pv_mmu_ops.flush_tlb_others;
	symbol_native_tlb_flush = (void *)kallsyms_lookup_name("native_flush_tlb_others");

	my_pr_info("tlb_flush_func(%lx), native_flush_tlb_others(%lx)\n", 
			(unsigned long )tlb_flush_func,
			(unsigned long )symbol_native_tlb_flush);

	if (skip_modify()) {
		goto end;
	}
	my_pr_info("modify func to  native_flush_tlb_others\n");
	modify_pv_tlb_ptr(symbol_native_tlb_flush);

end:	
	return 0;
}

static void __exit my_exit(void)
{
	if (!symbol_native_tlb_flush && is_native_tlb_flush()) {
		return;
	}

	modify_pv_tlb_ptr(tlb_flush_func);

        return;
}
module_param(debug, bool, 0);
MODULE_PARM_DESC(debug, "print debug");

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
