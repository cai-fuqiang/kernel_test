#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>
#include <sched.h>
#include <errno.h>

/* 缓存行大小（x86 通常为 64 字节） */
#define CACHE_LINE_SIZE 64

/* ==================== TAS 自旋锁 ==================== */
typedef struct {
    atomic_flag flag;
} tas_lock_t;

static inline void tas_lock_init(tas_lock_t *lock) {
    atomic_flag_clear(&lock->flag);
}

static inline void tas_lock(tas_lock_t *lock) {
    while (atomic_flag_test_and_set_explicit(&lock->flag, memory_order_acquire)) {
        #if defined(__x86_64__) || defined(__i386__)
        __asm__ volatile ("pause");
        #endif
    }
}

static inline void tas_unlock(tas_lock_t *lock) {
    atomic_flag_clear_explicit(&lock->flag, memory_order_release);
}

/* ==================== MCS 自旋锁 ==================== */
struct mcs_node;
typedef _Atomic(struct mcs_node *) atomic_node_ptr;

/* 每个节点按缓存行对齐，并填充至缓存行大小，避免伪共享 */
typedef struct mcs_node {
    atomic_node_ptr next;
    atomic_int      locked;
    /* 填充字段，使结构体大小 = CACHE_LINE_SIZE */
    char pad[CACHE_LINE_SIZE - sizeof(atomic_node_ptr) - sizeof(atomic_int)];
} mcs_node_t __attribute__((aligned(CACHE_LINE_SIZE)));

typedef struct mcs_lock {
    atomic_node_ptr tail;
} mcs_lock_t;

static inline void mcs_lock_init(mcs_lock_t *lock) {
    atomic_store_explicit(&lock->tail, NULL, memory_order_relaxed);
}

static inline void mcs_lock(mcs_lock_t *lock, mcs_node_t *node) {
    atomic_store_explicit(&node->next, NULL, memory_order_relaxed);
    atomic_store_explicit(&node->locked, 1, memory_order_relaxed);

    mcs_node_t *prev = atomic_exchange_explicit(&lock->tail, node,
                                                memory_order_acq_rel);
    if (prev != NULL) {
        atomic_store_explicit(&prev->next, node, memory_order_release);
        while (atomic_load_explicit(&node->locked, memory_order_acquire) != 0) {
            #if defined(__x86_64__) || defined(__i386__)
            __asm__ volatile ("pause");
            #endif
        }
    }
}

static inline void mcs_unlock(mcs_lock_t *lock, mcs_node_t *node) {
    mcs_node_t *next = atomic_load_explicit(&node->next, memory_order_acquire);
    if (next == NULL) {
        mcs_node_t *expected = node;
        if (atomic_compare_exchange_strong_explicit(
                &lock->tail, &expected, NULL,
                memory_order_release, memory_order_relaxed)) {
            return;
        }
        while ((next = atomic_load_explicit(&node->next,
                                            memory_order_acquire)) == NULL) {
            #if defined(__x86_64__) || defined(__i386__)
            __asm__ volatile ("pause");
            #endif
        }
    }
    atomic_store_explicit(&next->locked, 0, memory_order_release);
}

/* ==================== 测试框架 ==================== */
#define MAX_THREADS 1024

/* 全局锁变量 */
static tas_lock_t global_tas_lock;
static mcs_lock_t global_mcs_lock;
static mcs_node_t mcs_nodes[MAX_THREADS];  // 每个线程一个节点，已对齐

static volatile unsigned long counter = 0;

typedef struct {
    int thread_id;
    int iterations;
    int lock_type;  // 0: TAS, 1: MCS
} thread_arg_t;

void *worker(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    int tid = targ->thread_id;

    for (int i = 0; i < targ->iterations; i++) {
        if (targ->lock_type == 0) {
            tas_lock(&global_tas_lock);
            counter++;
            tas_unlock(&global_tas_lock);
        } else {
            mcs_lock(&global_mcs_lock, &mcs_nodes[tid]);
            counter++;
            mcs_unlock(&global_mcs_lock, &mcs_nodes[tid]);
        }
    }
    return NULL;
}

double run_test(int num_threads, int iterations, int lock_type) {
    pthread_t threads[MAX_THREADS];
    thread_arg_t args[MAX_THREADS];

    if (num_threads > MAX_THREADS) {
        fprintf(stderr, "Error: too many threads (max %d)\n", MAX_THREADS);
        exit(EXIT_FAILURE);
    }

    counter = 0;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].iterations = iterations;
        args[i].lock_type = lock_type;
        if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;
    return elapsed;
}

int main(int argc, char *argv[]) {
    int num_threads = 16;      // 默认线程数
    int iterations = 1000000;  // 每个线程默认迭代次数

    if (argc >= 2) num_threads = atoi(argv[1]);
    if (argc >= 3) iterations = atoi(argv[2]);

    printf("Testing TAS vs MCS spinlock with %d threads, %d iterations per thread\n\n",
           num_threads, iterations);

    // 初始化锁
    tas_lock_init(&global_tas_lock);
    mcs_lock_init(&global_mcs_lock);
    for (int i = 0; i < MAX_THREADS; i++) {
        atomic_store_explicit(&mcs_nodes[i].next, NULL, memory_order_relaxed);
        atomic_store_explicit(&mcs_nodes[i].locked, 0, memory_order_relaxed);
    }

    const char *lock_names[] = {"TAS spinlock", "MCS spinlock"};
    double total_ops = (double)num_threads * iterations;

    for (int lock_type = 0; lock_type < 2; lock_type++) {
        double elapsed = run_test(num_threads, iterations, lock_type);
        double throughput = total_ops / elapsed;
        double avg_latency_ns = (elapsed / total_ops) * 1e9;
        printf("%-14s : elapsed = %.6f s, throughput = %.2f Mops/s, avg latency = %.2f ns\n",
               lock_names[lock_type], elapsed, throughput / 1e6, avg_latency_ns);
    }

    return 0;
}

