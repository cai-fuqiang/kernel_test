// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <linux/sched.h>
#include <linux/ktime.h>
#include <linux/mutex.h>
#include <linux/limits.h>

#define PROC_DIR  "ipi_latency"
#define PROC_TRIG "trigger"
#define PROC_RES  "result"

struct ipi_result {
	u64 t_send;
	u64 latency_ns;
	u64 min_ns;
	u64 max_ns;
	u64 sum_ns;
	u32 count;
};

static struct ipi_result results[NR_CPUS];
static int result_last_dst = -1;
static DEFINE_MUTEX(trigger_mutex);

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_trig;
static struct proc_dir_entry *proc_res;

static void ipi_handler(void *info)
{
	u64 t_recv = ktime_get_ns();
	int dst = smp_processor_id();

	results[dst].latency_ns = t_recv - results[dst].t_send;
}

static ssize_t trigger_write(struct file *file, const char __user *buf,
			     size_t len, loff_t *ppos)
{
	char kbuf[64];
	int src, dst, count, i, ret = 0;
	struct cpumask mask;
	u64 lat;

	if (len >= sizeof(kbuf))
		return -EINVAL;
	if (copy_from_user(kbuf, buf, len))
		return -EFAULT;
	kbuf[len] = '\0';

	if (sscanf(kbuf, "%d %d %d", &src, &dst, &count) != 3)
		return -EINVAL;
	if (src < 0 || src >= nr_cpu_ids || !cpu_online(src))
		return -EINVAL;
	if (dst < 0 || dst >= nr_cpu_ids || !cpu_online(dst))
		return -EINVAL;
	if (src == dst)
		return -EINVAL;
	if (count <= 0 || count > 100000)
		return -EINVAL;

	if (mutex_lock_interruptible(&trigger_mutex))
		return -ERESTARTSYS;

	results[dst].min_ns = U64_MAX;
	results[dst].max_ns = 0;
	results[dst].sum_ns = 0;
	results[dst].count  = 0;

	cpumask_clear(&mask);
	cpumask_set_cpu(src, &mask);
	ret = set_cpus_allowed_ptr(current, &mask);
	if (ret)
		goto out;

	for (i = 0; i < count; i++) {
		results[dst].t_send = ktime_get_ns();
		smp_call_function_single(dst, ipi_handler, NULL, 1);
		lat = results[dst].latency_ns;
		if (lat < results[dst].min_ns) results[dst].min_ns = lat;
		if (lat > results[dst].max_ns) results[dst].max_ns = lat;
		results[dst].sum_ns += lat;
		results[dst].count++;
	}

	result_last_dst = dst;
	set_cpus_allowed_ptr(current, cpu_all_mask);

out:
	mutex_unlock(&trigger_mutex);
	return ret ? ret : (ssize_t)len;
}

static ssize_t result_read(struct file *file, char __user *buf,
			   size_t len, loff_t *ppos)
{
	char kbuf[64];
	int n, dst;
	u64 avg;

	if (*ppos > 0)
		return 0;

	mutex_lock(&trigger_mutex);
	dst = result_last_dst;
	if (dst < 0 || results[dst].count == 0) {
		mutex_unlock(&trigger_mutex);
		return -ENODATA;
	}
	avg = results[dst].sum_ns / results[dst].count;
	n = snprintf(kbuf, sizeof(kbuf), "%llu %llu %llu\n",
		     results[dst].min_ns, results[dst].max_ns, avg);
	mutex_unlock(&trigger_mutex);

	if ((size_t)n >= len)
		return -ERANGE;
	if (copy_to_user(buf, kbuf, n))
		return -EFAULT;
	*ppos += n;
	return n;
}

static const struct proc_ops trigger_ops = {
	.proc_write = trigger_write,
};

static const struct proc_ops result_ops = {
	.proc_read = result_read,
};

static int __init ipi_lat_init(void)
{
	proc_dir = proc_mkdir(PROC_DIR, NULL);
	if (!proc_dir)
		return -ENOMEM;

	proc_trig = proc_create(PROC_TRIG, 0200, proc_dir, &trigger_ops);
	if (!proc_trig) {
		proc_remove(proc_dir);
		return -ENOMEM;
	}

	proc_res = proc_create(PROC_RES, 0444, proc_dir, &result_ops);
	if (!proc_res) {
		proc_remove(proc_dir);
		return -ENOMEM;
	}

	pr_info("ipi_latency: loaded, /proc/%s/{%s,%s} ready\n",
		PROC_DIR, PROC_TRIG, PROC_RES);
	return 0;
}

static void __exit ipi_lat_exit(void)
{
	proc_remove(proc_dir);
	pr_info("ipi_latency: unloaded\n");
}

module_init(ipi_lat_init);
module_exit(ipi_lat_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("IPI end-to-end latency measurement (ARM64)");
