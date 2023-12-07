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
		.old_name = "kvm_flush_tlb_others",
		.new_func = livepatch_kvm_flush_tlb_others,
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
			goto end;  \
		} \
	     	pr_info("the %s (%lx) \n", #_symbol_name, (unsigned long)p__##_symbol_name); \
	}while(0)

static int get_p_symbol(void)
{
	int ret = 0;
	my_get_kallsyms(native_flush_tlb_others);
		
end:
	return ret;;
}


static int livepatch_init(void)
{
	int ret = 0;

	ret = get_p_symbol();
	if (ret)
		goto end;
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
