# IPI Latency Measurement Tool — Design Spec

**Date:** 2026-06-11  
**Platform:** ARM64  
**Status:** Approved

## Goals

Measure end-to-end IPI communication latency between all CPU pairs on ARM64. End-to-end means: from the moment the sender calls `smp_call_function_single()` (with timestamp taken just before) to the moment the IPI handler begins executing on the target CPU (timestamp taken as first instruction).

Output: N×N latency matrix printed to terminal (ns), with min/max/avg per pair.

## Architecture

```
User space (ipi_latency_test)         Kernel module (ipi_latency.ko)
─────────────────────────────         ──────────────────────────────
for src in 0..ncpus:                  /proc/ipi_latency/trigger  ← write "src dst count"
  for dst in 0..ncpus:               /proc/ipi_latency/result   ← read "min max avg" (ns)
    if src == dst: skip
    write "src dst N" → trigger
    read result
    record[src][dst] = {min,max,avg}
print matrix
```

## Kernel Module Design

### Data Structures

```c
struct ipi_result {
    u64 t_send;       /* timestamp before smp_call_function_single(), set by sender */
    u64 latency_ns;   /* last sample: t_recv - t_send */
    u64 min_ns;
    u64 max_ns;
    u64 sum_ns;
    u32 count;
};

/* One slot per target CPU; sender writes t_send, handler writes the rest */
static struct ipi_result results[NR_CPUS];
static int current_src;  /* protected by mutex */
static DEFINE_MUTEX(trigger_mutex);
```

### Measurement Loop (procfs write handler)

```
parse "src dst count" from user write
migrate sender thread to src CPU via set_cpus_allowed_ptr()
for i in 0..count:
    results[dst].t_send = ktime_get_ns()
    smp_call_function_single(dst, ipi_handler, NULL, 1)  /* wait=1 */
    /* handler has already written t_recv and computed latency */
accumulate min/max/sum into results[dst]
```

### IPI Handler (runs on target CPU)

```c
static void ipi_handler(void *info)
{
    u64 t_recv = ktime_get_ns();
    int dst = smp_processor_id();
    u64 lat = t_recv - results[dst].t_send;
    results[dst].latency_ns = lat;
    /* accumulation done by sender after wait=1 returns */
}
```

Note: `wait=1` ensures handler completes before `smp_call_function_single()` returns, so the sender safely reads `latency_ns` after the call.

### procfs Interface

| File | Direction | Format |
|------|-----------|--------|
| `/proc/ipi_latency/trigger` | write | `"<src> <dst> <count>\n"` |
| `/proc/ipi_latency/result`  | read  | `"<min_ns> <max_ns> <avg_ns>\n"` |

## User-Space Tool Design

```
Usage: ipi_latency_test [-n <samples>]
  -n  samples per CPU pair (default: 100)
```

Flow:
1. Read `nprocs` from `/proc/cpuinfo` or `sysconf(_SC_NPROCESSORS_ONLN)`
2. For each `(src, dst)` pair where `src != dst`:
   - Write `"src dst N"` to trigger
   - Read result → parse min/max/avg
3. Print N×N matrix; diagonal = `"-"`

### Output Format

```
IPI Latency Matrix - avg (ns) [100 samples]
       CPU0   CPU1   CPU2   CPU3
CPU0      -    342    389    401
CPU1    335      -    412    378
CPU2    401    388      -    356
CPU3    412    379    341      -

Min/Max per pair available via -v flag.
```

## File Structure

```
ipi_latency/
├── ipi_latency.c         # kernel module
├── ipi_latency_test.c    # user-space control program
└── Makefile              # builds both; KDIR defaults to /lib/modules/$(uname -r)/build
```

## Makefile Targets

```makefile
all: module userspace
module: ipi_latency.ko
userspace: ipi_latency_test
clean: remove .ko, .o, binary
```

## Error Handling

- Invalid `src`/`dst` (out of range, offline CPU): procfs write returns `-EINVAL`
- `src == dst`: procfs write returns `-EINVAL`
- `set_cpus_allowed_ptr` failure: propagate error to user
- User-space: check all read/write return values; print error and exit on failure

## Constraints

- ARM64 only; uses `ktime_get_ns()` (generic, available on all ARM64 kernels)
- Requires `CONFIG_SMP=y` (standard on ARM64 server/embedded platforms)
- `wait=1` on `smp_call_function_single` serializes measurement — intentional for accuracy, not a throughput benchmark
- Module must be loaded as root; user-space tool must run as root (for `set_cpus_allowed_ptr` via procfs)
