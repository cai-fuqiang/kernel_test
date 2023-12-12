#include <kvm/iodev.h>

#include <linux/kvm_host.h>
#include <linux/kvm.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/percpu.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/reboot.h>
#include <linux/debugfs.h>
#include <linux/highmem.h>
#include <linux/file.h>
#include <linux/syscore_ops.h>
#include <linux/cpu.h>
#include <linux/sched/signal.h>
#include <linux/sched/mm.h>
#include <linux/sched/stat.h>
#include <linux/cpumask.h>
#include <linux/smp.h>
#include <linux/anon_inodes.h>
#include <linux/profile.h>
#include <linux/kvm_para.h>
#include <linux/pagemap.h>
#include <linux/mman.h>
#include <linux/swap.h>
#include <linux/bitops.h>
#include <linux/spinlock.h>
#include <linux/compat.h>
#include <linux/srcu.h>
#include <linux/hugetlb.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/bsearch.h>
#include <linux/io.h>
#include <linux/lockdep.h>
#include <linux/kthread.h>
#include <linux/suspend.h>

#include <asm/processor.h>
#include <asm/ioctl.h>
#include <linux/uaccess.h>
#include <asm/pgtable.h>

struct kvm_host_map {
        /*
         * Only valid if the 'pfn' is managed by the host kernel (i.e. There is
         * a 'struct page' for it. When using mem= kernel parameter some memory
         * can be used as guest memory but they are not managed by host
         * kernel).
         * If 'pfn' is not managed by the host kernel, this field is
         * initialized to KVM_UNMAPPED_PAGE.
         */
        struct page *page;
        void *hva;
        kvm_pfn_t pfn;
        kvm_pfn_t gfn;
};

struct gfn_to_pfn_cache {
        u64 generation;
        gfn_t gfn;
        kvm_pfn_t pfn;
        bool dirty;
}

static void __kvm_unmap_gfn(struct kvm *kvm,
                        struct kvm_memory_slot *memslot,
                        struct kvm_host_map *map,
                        struct gfn_to_pfn_cache *cache,
                        bool dirty, bool atomic)
{
        if (!map)
                return;

        if (!map->hva)
                return;

        if (map->page != KVM_UNMAPPED_PAGE) {
                if (atomic)
                        kunmap_atomic(map->hva);
                else
                        kunmap(map->page);
        }
#ifdef CONFIG_HAS_IOMEM
        else if (!atomic)
                memunmap(map->hva);
        else
                WARN_ONCE(1, "Unexpected unmapping in atomic context");
#endif

        if (dirty)
                mark_page_dirty_in_slot(kvm, memslot, map->gfn);

        if (cache)
                cache->dirty |= dirty;
        else
                kvm_release_pfn(map->pfn, dirty, NULL);

        map->hva = NULL;
        map->page = NULL;
}

static int kvm_unmap_gfn(struct kvm_vcpu *vcpu, struct kvm_host_map *map,
                  struct gfn_to_pfn_cache *cache, bool dirty, bool atomic)
{
        __kvm_unmap_gfn(vcpu->kvm, gfn_to_memslot(vcpu->kvm, map->gfn), map,
                        cache, dirty, atomic);
        return 0;
}


static int __kvm_map_gfn(struct kvm_memslots *slots, gfn_t gfn,
                         struct kvm_host_map *map,
                         struct gfn_to_pfn_cache *cache,
                         bool atomic)
{
        kvm_pfn_t pfn;
        void *hva = NULL;
        struct page *page = KVM_UNMAPPED_PAGE;
        struct kvm_memory_slot *slot = __gfn_to_memslot(slots, gfn);
        u64 gen = slots->generation;

        if (!map)
                return -EINVAL;

        if (cache) {
                if (!cache->pfn || cache->gfn != gfn ||
                        cache->generation != gen) {
                        if (atomic)
                                return -EAGAIN;
                        kvm_cache_gfn_to_pfn(slot, gfn, cache, gen);
                }
                pfn = cache->pfn;
        } else {
                if (atomic)
                        return -EAGAIN;
                pfn = gfn_to_pfn_memslot(slot, gfn);
        }
        if (is_error_noslot_pfn(pfn))
                return -EINVAL;

        if (pfn_valid(pfn)) {
                page = pfn_to_page(pfn);
                if (atomic)
                        hva = kmap_atomic(page);
                else
                        hva = kmap(page);
#ifdef CONFIG_HAS_IOMEM
        } else if (!atomic) {
                hva = memremap(pfn_to_hpa(pfn), PAGE_SIZE, MEMREMAP_WB);
        } else {
                return -EINVAL;
#endif
        }

        if (!hva)
                return -EFAULT;

        map->page = page;
        map->hva = hva;
        map->pfn = pfn;
        map->gfn = gfn;

        return 0;
}

static int kvm_map_gfn(struct kvm_vcpu *vcpu, gfn_t gfn, struct kvm_host_map *map,
                struct gfn_to_pfn_cache *cache, bool atomic)
{
        return __kvm_map_gfn(kvm_memslots(vcpu->kvm), gfn, map,
                        cache, atomic);
}

void livepatch_record_steal_time(struct kvm_vcpu *vcpu)
{
	struct kvm_host_map map;
	struct kvm_steal_time *st;

	struct gfn_to_pfn_cache cache;

	memset(&cache, 0, sizeof(cache));

        if (!(vcpu->arch.st.msr_val & KVM_MSR_ENABLED))
                return;

        /* -EAGAIN is returned in atomic context so we can just return. */
        if (kvm_map_gfn(vcpu, vcpu->arch.st.msr_val >> PAGE_SHIFT,
                        &map, &cache, false))
                return;

        st = map.hva +
                offset_in_page(vcpu->arch.st.msr_val & KVM_STEAL_VALID_BITS);

        /*
         * Doing a TLB flush here, on the guest's behalf, can avoid
         * expensive IPIs.
         */
	if (xchg(&st->preempted, 0) & KVM_VCPU_FLUSH_TLB)
                kvm_vcpu_flush_tlb(vcpu, false);
	
	vcpu->arch.st.steal.preempted = 0;
	if (st->version & 1)
		st->version += 1;

	st->version += 1;

        smp_wmb();

	st->steal += current->sched_info.run_delay -
                vcpu->arch.st.last_steal;
        vcpu->arch.st.last_steal = current->sched_info.run_delay;

        smp_wmb();

	st->version += 1;

	kvm_unmap_gfn(vcpu, &map, &vcpu->arch.st.cache, true, false);
}
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

//#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/livepatch.h>
//#include <linux/mm.h>
//#include <linux/spinlock.h>
//#include <linux/smp.h>
#include <linux/interrupt.h>
#include <linux/export.h>
//#include <linux/cpu.h>
//#include <linux/debugfs.h>

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

static void (*p__native_flush_tlb_others)(
		const struct cpumask *cpumask,
		const struct flush_tlb_info *info);

static void livepatch_kvm_flush_tlb_others(const struct cpumask *cpumask,
		                        const struct flush_tlb_info *info)
{
	return p__native_flush_tlb_others(cpumask, info);
}

static struct klp_func funcs[] = {
	{
		.old_name = "record_steal_time",
		.new_func = livepatch_record_steal_time,
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

static int livepatch_init(void)
{
	int ret = 0;

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
end:
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
