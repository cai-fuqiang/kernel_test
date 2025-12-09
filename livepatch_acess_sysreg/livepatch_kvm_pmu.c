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
#include <linux/bits.h>

#include <asm/mmu_context.h>
#include <asm/cache.h>

#include <linux/kallsyms.h>
//#define MY_PATCH
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
#include <linux/kvm_host.h>
#include <asm/cpufeature.h>

#define ID_DFR0_PERFMON_8_1     0x4
#define ID_AA64DFR0_PMUVER_8_1      0x4
#define ID_DFR0_PERFMON_SHIFT       24

struct sys_reg_params {
    u8  Op0;
    u8  Op1;
    u8  CRn;
    u8  CRm;
    u8  Op2;
    u64 regval;
    bool    is_write;
    bool    is_aarch32;
    bool    is_32bit;   /* Only valid if is_aarch32 is true */
};

struct sys_reg_desc {
        const char *name;
        u8  Op0;
        u8  Op1;
        u8  CRn;
        u8  CRm;
        u8  Op2;
        void *access;
        void *reset;
        int reg;
        u64 val;
        void *get_user;
        void *set_user;
        unsigned int (*visibility)(const struct kvm_vcpu *vcpu,
                const struct sys_reg_desc *rd);
};

#define REG_RAZ (1 << 1)

/*
 * Fields that identify the version of the Performance Monitors Extension do
 * not follow the standard ID scheme. See ARM DDI 0487E.a page D13-2825,
 * "Alternative ID scheme used for the Performance Monitors Extension version".
 */
static inline u64 __attribute_const__
cpuid_feature_cap_perfmon_field(u64 features, int field, u64 cap)
{
    u64 val = cpuid_feature_extract_unsigned_field(features, field);
    u64 mask = GENMASK_ULL(field + 3, field);

    /* Treat IMPLEMENTATION DEFINED functionality as unimplemented */
    if (val == 0xf)
        val = 0;

    if (val > cap) {
        features &= ~mask;
        features |= (cap << field) & mask;
    }

    return features;
}

static bool (*p__read_id_reg)(struct sys_reg_desc *r, bool raz);
static u64 (*p__read_sanitised_ftr_reg)(u32 id);
static bool (*p__write_to_read_only)(struct kvm_vcpu *vcpu,
                struct sys_reg_params *p,
                struct sys_reg_desc *r);
static bool livepatch_access_id_reg(struct kvm_vcpu *vcpu,
                struct sys_reg_params *p,
                struct sys_reg_desc *r)
{
    u32 id = sys_reg((u32)r->Op0, (u32)r->Op1,
             (u32)r->CRn, (u32)r->CRm, (u32)r->Op2);

    u64 val = 0;

    pr_info("r is %lx\n", (unsigned long) r);

    if (p->is_write) {
        return p__write_to_read_only(vcpu, p, r);
    }

    pr_info("Op0(%u) Op1(%u) CRn(%u) CRm(%u) Op2(%u)",
                    p->Op0, p->Op1, p->CRn, p->CRm, p->Op2);
    pr_info("Op0(%u) Op1(%u) CRn(%u) CRm(%u) Op2(%u)",
                    r->Op0, r->Op1, r->CRn, r->CRm, r->Op2);

    val =  p__read_sanitised_ftr_reg(id);
    pr_info("val (%lx) \n", (unsigned long)val);

    if (id == SYS_ID_AA64PFR0_EL1) {
        if (val & (0xfUL << ID_AA64PFR0_SVE_SHIFT))
            kvm_debug("SVE unsupported for guests, suppressing\n");

        val &= ~(0xfUL << ID_AA64PFR0_SVE_SHIFT);
    } else if (id == SYS_ID_AA64MMFR1_EL1) {
        if (val & (0xfUL << ID_AA64MMFR1_LOR_SHIFT))
            kvm_debug("LORegions unsupported for guests, suppressing\n");

        val &= ~(0xfUL << ID_AA64MMFR1_LOR_SHIFT);
    } if (id == SYS_ID_AA64DFR0_EL1) {
        /* Limit guests to PMUv3 for ARMv8.1 */
        val = cpuid_feature_cap_perfmon_field(val,
                        ID_AA64DFR0_PMUVER_SHIFT,
                        ID_AA64DFR0_PMUVER_8_1);
    } else if (id == SYS_ID_DFR0_EL1) {
        /* Limit guests to PMUv3 for ARMv8.1 */
        val = cpuid_feature_cap_perfmon_field(val,
                        ID_DFR0_PERFMON_SHIFT,
                        ID_DFR0_PERFMON_8_1);
    }

    p->regval = val;
    return true;
}

static struct klp_func funcs[] = {
        {
                .old_name = "access_id_reg",
                .new_func = livepatch_access_id_reg,
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

#define my_get_kallsyms(_symbol_name, _var_name)        \
        do {  \
                p__##_var_name = (typeof(p__##_var_name))kallsyms_lookup_name(#_symbol_name);  \
                if (!p__##_var_name) {  \
                        pr_info("get %s ptr error\n", #_symbol_name);  \
                        ret = -1;  \
                        goto end;  \
                } \
                pr_info("the %s (%lx) \n", #_symbol_name, (unsigned long)p__##_var_name); \
        } while(0)


static int get_p_symbol(void)
{
        int ret = 0;
        my_get_kallsyms(read_id_reg.isra.23, read_id_reg);
        my_get_kallsyms(read_sanitised_ftr_reg, read_sanitised_ftr_reg);
        my_get_kallsyms(write_to_read_only, write_to_read_only);

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
