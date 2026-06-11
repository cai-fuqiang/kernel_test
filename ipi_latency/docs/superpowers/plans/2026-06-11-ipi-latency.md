# IPI Latency Measurement Tool — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build an ARM64 kernel module + user-space tool that measures IPI end-to-end latency for all CPU pairs and prints an N×N matrix in nanoseconds.

**Architecture:** Kernel module exposes `/proc/ipi_latency/trigger` (write "src dst count") and `/proc/ipi_latency/result` (read "min max avg"). User-space tool iterates all CPU pairs, writes trigger, reads result, prints matrix.

**Tech Stack:** C (kernel module, user-space), Linux procfs API, `smp_call_function_single()`, `ktime_get_ns()`, `set_cpus_allowed_ptr()`.

---

## File Map

| File | Role |
|------|------|
| `ipi_latency/ipi_latency.c` | Kernel module: procfs, IPI measurement loop, handler |
| `ipi_latency/ipi_latency_test.c` | User-space: parse args, iterate CPU pairs, print matrix |
| `ipi_latency/Makefile` | Build both targets; `KDIR` defaults to `/lib/modules/$(shell uname -r)/build` |

---

## Task 1: Kernel Module — Skeleton + procfs dirs

**Files:**
- Create: `ipi_latency/ipi_latency.c`

- [ ] **Step 1: Write the module skeleton**

```c
// ipi_latency/ipi_latency.c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <linux/sched.h>
#include <linux/ktime.h>
#include <linux/mutex.h>

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
static DEFINE_MUTEX(trigger_mutex);

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_trig;
static struct proc_dir_entry *proc_res;

static int __init ipi_lat_init(void)
{
    proc_dir = proc_mkdir(PROC_DIR, NULL);
    if (!proc_dir)
        return -ENOMEM;
    return 0;
}

static void __exit ipi_lat_exit(void)
{
    proc_remove(proc_dir);  /* removes children too */
}

module_init(ipi_lat_init);
module_exit(ipi_lat_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("IPI end-to-end latency measurement");
```

- [ ] **Step 2: Create Makefile**

```makefile
# ipi_latency/Makefile
KDIR ?= /lib/modules/$(shell uname -r)/build

obj-m := ipi_latency.o

all: module userspace

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

userspace: ipi_latency_test.c
	$(CC) -O2 -o ipi_latency_test ipi_latency_test.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f ipi_latency_test
```

- [ ] **Step 3: Verify module compiles (no link yet)**

```bash
cd /Users/wangfuqiang49/workspace/kernel_test/ipi_latency
make module KDIR=/export/wfq/kernel/linux-6.16.9
```

Expected: `ipi_latency.ko` built, no errors.

- [ ] **Step 4: Commit**

```bash
git add ipi_latency/ipi_latency.c ipi_latency/Makefile
git commit -m "feat: ipi_latency kmod skeleton + Makefile"
```

---

## Task 2: Kernel Module — IPI handler + trigger write handler

**Files:**
- Modify: `ipi_latency/ipi_latency.c`

- [ ] **Step 1: Add IPI handler function** (before `ipi_lat_init`)

```c
static void ipi_handler(void *info)
{
    u64 t_recv = ktime_get_ns();
    int dst = smp_processor_id();
    u64 lat;

    lat = t_recv - results[dst].t_send;
    results[dst].latency_ns = lat;
}
```

- [ ] **Step 2: Add trigger write handler** (before `ipi_lat_init`)

```c
static ssize_t trigger_write(struct file *file, const char __user *buf,
                              size_t len, loff_t *ppos)
{
    char kbuf[64];
    int src, dst, count, i, ret;
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

    /* Reset stats for this dst slot */
    results[dst].min_ns = U64_MAX;
    results[dst].max_ns = 0;
    results[dst].sum_ns = 0;
    results[dst].count  = 0;

    /* Pin this kernel thread to src CPU */
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

    /* Restore affinity to all CPUs */
    set_cpus_allowed_ptr(current, cpu_all_mask);

out:
    mutex_unlock(&trigger_mutex);
    return ret ? ret : len;
}

static const struct proc_ops trigger_ops = {
    .proc_write = trigger_write,
};
```

- [ ] **Step 3: Register trigger in init**

Replace the `return 0;` in `ipi_lat_init` with:

```c
    proc_trig = proc_create(PROC_TRIG, 0200, proc_dir, &trigger_ops);
    if (!proc_trig) {
        proc_remove(proc_dir);
        return -ENOMEM;
    }
    return 0;
```

- [ ] **Step 4: Rebuild and verify**

```bash
make module KDIR=/export/wfq/kernel/linux-6.16.9
```

Expected: compiles clean.

- [ ] **Step 5: Commit**

```bash
git add ipi_latency/ipi_latency.c
git commit -m "feat: ipi_latency kmod trigger write handler + IPI measurement loop"
```

---

## Task 3: Kernel Module — result read handler

**Files:**
- Modify: `ipi_latency/ipi_latency.c`

- [ ] **Step 1: Add result read handler** (before `ipi_lat_init`)

```c
/* Userspace reads last result set by trigger_write */
static int result_last_dst = -1;  /* set by trigger_write, read here */

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

    if (n >= len)
        return -ERANGE;
    if (copy_to_user(buf, kbuf, n))
        return -EFAULT;
    *ppos += n;
    return n;
}

static const struct proc_ops result_ops = {
    .proc_read = result_read,
};
```

- [ ] **Step 2: Set `result_last_dst` in trigger_write**

In `trigger_write`, just before `mutex_unlock(&trigger_mutex)` in the non-error path, add:

```c
    result_last_dst = dst;
```

Full context — add it right after the measurement loop ends and before `set_cpus_allowed_ptr(current, cpu_all_mask)`:

```c
    result_last_dst = dst;
    /* Restore affinity to all CPUs */
    set_cpus_allowed_ptr(current, cpu_all_mask);
```

- [ ] **Step 3: Register result proc entry in init**

After `proc_trig` creation in `ipi_lat_init`:

```c
    proc_res = proc_create(PROC_RES, 0444, proc_dir, &result_ops);
    if (!proc_res) {
        proc_remove(proc_dir);
        return -ENOMEM;
    }
```

- [ ] **Step 4: Rebuild**

```bash
make module KDIR=/export/wfq/kernel/linux-6.16.9
```

Expected: compiles clean.

- [ ] **Step 5: Commit**

```bash
git add ipi_latency/ipi_latency.c
git commit -m "feat: ipi_latency kmod result read handler"
```

---

## Task 4: User-Space Tool

**Files:**
- Create: `ipi_latency/ipi_latency_test.c`

- [ ] **Step 1: Write the user-space tool**

```c
// ipi_latency/ipi_latency_test.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define TRIG_PATH "/proc/ipi_latency/trigger"
#define RES_PATH  "/proc/ipi_latency/result"

static int measure(int src, int dst, int n,
                   unsigned long long *min_ns,
                   unsigned long long *max_ns,
                   unsigned long long *avg_ns)
{
    char buf[64];
    int fd, len, ret;

    fd = open(TRIG_PATH, O_WRONLY);
    if (fd < 0) {
        perror("open trigger");
        return -1;
    }
    len = snprintf(buf, sizeof(buf), "%d %d %d\n", src, dst, n);
    ret = write(fd, buf, len);
    close(fd);
    if (ret < 0) {
        perror("write trigger");
        return -1;
    }

    fd = open(RES_PATH, O_RDONLY);
    if (fd < 0) {
        perror("open result");
        return -1;
    }
    len = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (len <= 0) {
        perror("read result");
        return -1;
    }
    buf[len] = '\0';

    if (sscanf(buf, "%llu %llu %llu", min_ns, max_ns, avg_ns) != 3) {
        fprintf(stderr, "bad result format: %s\n", buf);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int n = 100;
    int opt;
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        if (opt == 'n')
            n = atoi(optarg);
        else {
            fprintf(stderr, "Usage: %s [-n samples]\n", argv[0]);
            return 1;
        }
    }

    int ncpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (ncpus <= 0) {
        fprintf(stderr, "sysconf failed\n");
        return 1;
    }

    /* avg[src][dst] = 0 means same CPU (skip) */
    unsigned long long avg[ncpus][ncpus];
    memset(avg, 0, sizeof(avg));

    for (int src = 0; src < ncpus; src++) {
        for (int dst = 0; dst < ncpus; dst++) {
            unsigned long long mn, mx, av;
            if (src == dst) continue;
            if (measure(src, dst, n, &mn, &mx, &av) < 0) {
                fprintf(stderr, "FAIL cpu%d->cpu%d\n", src, dst);
                return 1;
            }
            avg[src][dst] = av;
        }
    }

    /* Print matrix */
    printf("\nIPI Latency Matrix - avg (ns) [%d samples]\n", n);
    printf("%8s", "");
    for (int i = 0; i < ncpus; i++)
        printf("  CPU%-4d", i);
    printf("\n");

    for (int src = 0; src < ncpus; src++) {
        printf("CPU%-5d", src);
        for (int dst = 0; dst < ncpus; dst++) {
            if (src == dst)
                printf("       -");
            else
                printf("  %6llu", avg[src][dst]);
        }
        printf("\n");
    }
    printf("\n");
    return 0;
}
```

- [ ] **Step 2: Build userspace**

```bash
cd /Users/wangfuqiang49/workspace/kernel_test/ipi_latency
make userspace
```

Expected: `ipi_latency_test` binary created, no warnings.

- [ ] **Step 3: Commit**

```bash
git add ipi_latency/ipi_latency_test.c
git commit -m "feat: ipi_latency user-space test tool"
```

---

## Task 5: Integration Test (on ARM64 target)

**Pre-conditions:** ARM64 machine with `CONFIG_SMP=y`, kernel built at `KDIR`, run as root.

- [ ] **Step 1: Build both targets**

```bash
cd /path/to/ipi_latency
make KDIR=/export/wfq/kernel/linux-6.16.9
```

Expected: `ipi_latency.ko` and `ipi_latency_test` present.

- [ ] **Step 2: Load module**

```bash
insmod ipi_latency.ko
dmesg | tail -5
ls /proc/ipi_latency/
```

Expected: `trigger` and `result` appear under `/proc/ipi_latency/`.

- [ ] **Step 3: Manual smoke test (2-CPU pair)**

```bash
echo "0 1 10" > /proc/ipi_latency/trigger
cat /proc/ipi_latency/result
```

Expected output (example): `412 891 523` (min max avg in ns, values vary by hardware).

- [ ] **Step 4: Run full matrix tool**

```bash
./ipi_latency_test -n 100
```

Expected: N×N matrix printed, diagonal `-`, all other cells non-zero ns values.

- [ ] **Step 5: Test error cases**

```bash
# same CPU
echo "0 0 10" > /proc/ipi_latency/trigger
echo "exit code should indicate error: $?"

# out of range CPU
echo "0 999 10" > /proc/ipi_latency/trigger
echo "exit code should indicate error: $?"
```

Expected: both writes return error (errno EINVAL visible via `dmesg` or `strace`).

- [ ] **Step 6: Unload module**

```bash
rmmod ipi_latency
ls /proc/ipi_latency/ 2>&1
```

Expected: directory gone, `ls` reports no such file.

- [ ] **Step 7: Commit**

```bash
git add -p   # nothing new to stage; this is a verification step
git commit --allow-empty -m "test: ipi_latency integration verified on ARM64"
```

---

## Self-Review Checklist

- [x] **Spec coverage:** procfs interface ✓, IPI handler ✓, measurement loop with wait=1 ✓, set_cpus_allowed_ptr ✓, min/max/avg stats ✓, N×N matrix output ✓, error handling (invalid src/dst/same CPU) ✓, Makefile targets ✓
- [x] **No placeholders:** all steps have real code
- [x] **Type consistency:** `result_last_dst` set in Task 3 step 2, used in `result_read` defined in same task; `results[]` array used consistently across Tasks 1-3
- [x] **`U64_MAX`** requires `<linux/limits.h>` — add to includes in Task 1 step 1 if not pulled in transitively via `<linux/ktime.h>`. Safe to add explicitly.
