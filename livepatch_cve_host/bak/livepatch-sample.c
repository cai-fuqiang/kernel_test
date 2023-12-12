/*
 * livepatch-sample.c - Kernel Live Patching Sample Module
 *
 * Copyright (C) 2014 Seth Jennings <sjenning@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/livepatch.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/interrupt.h>
#include <linux/export.h>
#include <linux/cpu.h>
#include <linux/debugfs.h>

#include <asm/tlbflush.h>
#include <asm/mmu_context.h>
#include <asm/nospec-branch.h>
#include <asm/cache.h>
#include <asm/apic.h>
#include <asm/uv/uv.h>

#include <linux/kallsyms.h>

/*
 * This (dumb) live patch overrides the function that prints the
 * kernel boot cmdline when /proc/cmdline is read.
 *
 * Example:
 *
 * $ cat /proc/cmdline
 * <your cmdline>
 *
 * $ insmod livepatch-sample.ko
 * $ cat /proc/cmdline
 * this has been live patched
 *
 * $ echo 0 > /sys/kernel/livepatch/livepatch_sample/enabled
 * $ cat /proc/cmdline
 * <your cmdline>
 */

#include <linux/seq_file.h>
# if 0
static void flush_tlb_func_local(void *info, enum tlb_flush_reason reason)
{
	        const struct flush_tlb_info *f = info;

		        flush_tlb_func_common(f, true, reason);
}
static void flush_tlb_func_common(const struct flush_tlb_info *f,
                                  bool local, enum tlb_flush_reason reason)
{
        /*
         * We have three different tlb_gen values in here.  They are:
         *
         * - mm_tlb_gen:     the latest generation.
         * - local_tlb_gen:  the generation that this CPU has already caught
         *                   up to.
         * - f->new_tlb_gen: the generation that the requester of the flush
         *                   wants us to catch up to.
         */
        struct mm_struct *loaded_mm = this_cpu_read(cpu_tlbstate.loaded_mm);
        u32 loaded_mm_asid = this_cpu_read(cpu_tlbstate.loaded_mm_asid);
        u64 mm_tlb_gen = atomic64_read(&loaded_mm->context.tlb_gen);
        u64 local_tlb_gen = this_cpu_read(cpu_tlbstate.ctxs[loaded_mm_asid].tlb_gen);

        /* This code cannot presently handle being reentered. */
        VM_WARN_ON(!irqs_disabled());

        if (unlikely(loaded_mm == &init_mm))
                return;

        VM_WARN_ON(this_cpu_read(cpu_tlbstate.ctxs[loaded_mm_asid].ctx_id) !=
                   loaded_mm->context.ctx_id);

        if (this_cpu_read(cpu_tlbstate.is_lazy)) {
                /*
                 * We're in lazy mode.  We need to at least flush our
                 * paging-structure cache to avoid speculatively reading
                 * garbage into our TLB.  Since switching to init_mm is barely
                 * slower than a minimal flush, just switch to init_mm.
                 */
                switch_mm_irqs_off(NULL, &init_mm, NULL);
                return;
        }

        if (unlikely(local_tlb_gen == mm_tlb_gen)) {
                /*
                 * There's nothing to do: we're already up to date.  This can
                 * happen if two concurrent flushes happen -- the first flush to
                 * be handled can catch us all the way up, leaving no work for
                 * the second flush.
                 */
                trace_tlb_flush(reason, 0);
                return;
        }

        WARN_ON_ONCE(local_tlb_gen > mm_tlb_gen);
        WARN_ON_ONCE(f->new_tlb_gen > mm_tlb_gen);

        /*
         * If we get to this point, we know that our TLB is out of date.
         * This does not strictly imply that we need to flush (it's
         * possible that f->new_tlb_gen <= local_tlb_gen), but we're
         * going to need to flush in the very near future, so we might
         * as well get it over with.
         *
         * The only question is whether to do a full or partial flush.
         *
         * We do a partial flush if requested and two extra conditions
         * are met:
         *
         * 1. f->new_tlb_gen == local_tlb_gen + 1.  We have an invariant that
         *    we've always done all needed flushes to catch up to
         *    local_tlb_gen.  If, for example, local_tlb_gen == 2 and
         *    f->new_tlb_gen == 3, then we know that the flush needed to bring
         *    us up to date for tlb_gen 3 is the partial flush we're
         *    processing.
         *
         *    As an example of why this check is needed, suppose that there
         *    are two concurrent flushes.  The first is a full flush that
         *    changes context.tlb_gen from 1 to 2.  The second is a partial
         *    flush that changes context.tlb_gen from 2 to 3.  If they get
         *    processed on this CPU in reverse order, we'll see
         *     local_tlb_gen == 1, mm_tlb_gen == 3, and end != TLB_FLUSH_ALL.
         *    If we were to use __flush_tlb_one_user() and set local_tlb_gen to
         *    3, we'd be break the invariant: we'd update local_tlb_gen above
         *    1 without the full flush that's needed for tlb_gen 2.
         *
         * 2. f->new_tlb_gen == mm_tlb_gen.  This is purely an optimiation.
         *    Partial TLB flushes are not all that much cheaper than full TLB
         *    flushes, so it seems unlikely that it would be a performance win
         *    to do a partial flush if that won't bring our TLB fully up to
         *    date.  By doing a full flush instead, we can increase
         *    local_tlb_gen all the way to mm_tlb_gen and we can probably
         *    avoid another flush in the very near future.
         */
        if (f->end != TLB_FLUSH_ALL &&
            f->new_tlb_gen == local_tlb_gen + 1 &&
            f->new_tlb_gen == mm_tlb_gen) {
                /* Partial flush */
                unsigned long addr;
                unsigned long nr_pages = (f->end - f->start) >> PAGE_SHIFT;

                addr = f->start;
                while (addr < f->end) {
                        __flush_tlb_one_user(addr);
                        addr += PAGE_SIZE;
                }
                if (local)
                        count_vm_tlb_events(NR_TLB_LOCAL_FLUSH_ONE, nr_pages);
                trace_tlb_flush(reason, nr_pages);
        } else {
                /* Full flush. */
                local_flush_tlb();
                if (local)
                        count_vm_tlb_event(NR_TLB_LOCAL_FLUSH_ALL);
                trace_tlb_flush(reason, TLB_FLUSH_ALL);
        }

        /* Both paths above update our state to mm_tlb_gen. */
        this_cpu_write(cpu_tlbstate.ctxs[loaded_mm_asid].tlb_gen, mm_tlb_gen);
}
#endif

static unsigned long *p__tlb_single_page_flush_ceiling;
static void (*p__flush_tlb_func_common)(const struct flush_tlb_info *f,
		bool local, enum tlb_flush_reason reason);
static void (*p__native_flush_tlb_others)(
		const struct cpumask *cpumask,
		const struct flush_tlb_info *info);

static unsigned long *b__tlb_single_page_flush_ceiling = 0xffffffff82402488;
static void (*b__flush_tlb_func_common)(const struct flush_tlb_info *f,
		bool local, enum tlb_flush_reason reason)
	= 0xffffffff8106f6f0;
static void (*b__native_flush_tlb_others)(const struct cpumask *cpumask,
		const struct flush_tlb_info *info)
	= 0xffffffff8106faa0;

static void (*b____kmalloc)(size_t size, gfp_t flags) = 0xffffffff81288140;

static void flush_tlb_func_local(void *info, enum tlb_flush_reason reason)
{
        const struct flush_tlb_info *f = info;

        p__flush_tlb_func_common(f, true, reason);
}

void livepatch_flush_tlb_mm_range(struct mm_struct *mm, unsigned long start,
                                unsigned long end, unsigned long vmflag)
{
        int cpu;

        struct flush_tlb_info info __aligned(SMP_CACHE_BYTES) = {
                .mm = mm,
        };

        cpu = get_cpu();

        /* This is also a barrier that synchronizes with switch_mm(). */
        info.new_tlb_gen = inc_mm_tlb_gen(mm);

        /* Should we flush just the requested range? */
        if ((end != TLB_FLUSH_ALL) &&
            !(vmflag & VM_HUGETLB) &&
            ((end - start) >> PAGE_SHIFT) <= *p__tlb_single_page_flush_ceiling) {
                info.start = start;
                info.end = end;
        } else {
                info.start = 0UL;
                info.end = TLB_FLUSH_ALL;
        }

        if (mm == this_cpu_read(cpu_tlbstate.loaded_mm)) {
                VM_WARN_ON(irqs_disabled());
                local_irq_disable();
                flush_tlb_func_local(&info, TLB_LOCAL_MM_SHOOTDOWN);
                local_irq_enable();
        }

        if (cpumask_any_but(mm_cpumask(mm), cpu) < nr_cpu_ids)
                p__native_flush_tlb_others(mm_cpumask(mm), &info);

        put_cpu();
}

static struct klp_func funcs[] = {
	{
		.old_name = "flush_tlb_mm_range",
		.new_func = livepatch_flush_tlb_mm_range,
	}, { }
};

static struct klp_object objs[] = {
	{
		/* name being NULL means vmlinux */
		.funcs = funcs,
	}, { }
};

static struct klp_patch patch = {
	.mod = THIS_MODULE,
	.objs = objs,
};

#define my_get_kallsyms(_symbol_name)	\
	do {  \
		p__##_symbol_name = (typeof(p__##_symbol_name))kallsyms_lookup_name(#_symbol_name);  \
		if (!p__##_symbol_name) {  \
			pr_info("get %s ptr error\n", #_symbol_name);  \
			ret = -1;  \
			goto err;  \
		} \
	     	pr_info("the %s (%lx) \n", #_symbol_name, (unsigned long)p__##_symbol_name); \
	}while(0)

static unsigned long calculate_kaslr(void)
{
	int ret;
        typeof(b____kmalloc) p____kmalloc;

        p____kmalloc = kallsyms_lookup_name("__kmalloc");
        return p____kmalloc - b____kmalloc;
}

#define calculate_p_addr(_symbol_name, _offset) 	\
	do {	\
		p__##_symbol_name = (typeof(p__##_symbol_name))((char *)(b__##_symbol_name) + _offset);	\
	     	pr_info("the b__%s (%lx) \n", #_symbol_name, (unsigned long)b__##_symbol_name); \
	     	pr_info("the p__%s (%lx) \n", #_symbol_name, (unsigned long)p__##_symbol_name); \
	}while(0);

static void get_p_symbol(void)
{
	unsigned long offset;
	offset = calculate_kaslr();
	pr_info("offset is %lx\n", offset);
	calculate_p_addr(tlb_single_page_flush_ceiling, offset);
	calculate_p_addr(flush_tlb_func_common, offset);
	calculate_p_addr(native_flush_tlb_others, offset);
}


static int livepatch_init(void)
{
	int ret = 0;

	get_p_symbol();
#if 1
	ret = klp_register_patch(&patch);
	if (ret)
		return ret;
	ret = klp_enable_patch(&patch);
	if (ret) {
		WARN_ON(klp_unregister_patch(&patch));
		return ret;
	}
#endif
	return ret;
}

static void livepatch_exit(void)
{
#if 1
	WARN_ON(klp_unregister_patch(&patch));
#endif
	return ;
}

module_init(livepatch_init);
module_exit(livepatch_exit);
MODULE_LICENSE("GPL");
MODULE_INFO(livepatch, "Y");
