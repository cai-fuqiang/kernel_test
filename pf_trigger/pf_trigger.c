#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#define ALLOC_SIZE      (1024 * 1024)          // 1 MB
#define DEFAULT_TARGET  (4ULL * 1024 * 1024 * 1024) // 4 GB
#define MAX_PROCESSES   32

/* 获取当前时间（秒） */
static double get_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

/* 基准测试工作函数：反复分配、访问、释放1MB内存，直到累计访问量达到目标值 */
static int benchmark_worker(uint64_t target_bytes) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        perror("sysconf(_SC_PAGESIZE)");
        return -1;
    }

    uint64_t total_accessed = 0;
    int pages_per_alloc = ALLOC_SIZE / page_size;  // 1MB 中的页数（通常256）

    while (total_accessed < target_bytes) {
        // 分配1MB匿名内存
        void *addr = mmap(NULL, ALLOC_SIZE, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (addr == MAP_FAILED) {
            perror("mmap");
            return -1;
        }

        // 以页粒度访问：对每个页写入一个字节，确保页被实际映射
        volatile char *p = (volatile char *)addr;
        for (int i = 0; i < pages_per_alloc; i++) {
            p[i * page_size] = (char)(i & 0xff);  // 写页首字节
            //printf("the addr is %lx\n", (unsigned long *)(&p[i * page_size]));
        }

        // 释放内存
        if (munmap(addr, ALLOC_SIZE) == -1) {
            perror("munmap");
            return -1;
        }

        total_accessed += ALLOC_SIZE;  // 本次访问了1MB
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int num_processes = 1;
    uint64_t target_bytes = DEFAULT_TARGET;

    // 解析命令行参数：-n 进程数，-t 目标字节数（可选，默认4GB）
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            num_processes = atoi(argv[++i]);
            if (num_processes < 1 || num_processes > MAX_PROCESSES) {
                fprintf(stderr, "进程数必须在1到%d之间\n", MAX_PROCESSES);
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            target_bytes = strtoull(argv[++i], NULL, 10);
        } else {
            fprintf(stderr, "用法: %s [-n 进程数] [-t 目标字节数]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    printf("启动 %d 个进程，每个进程累计访问 %llu 字节（%.2f GB）\n",
           num_processes, (unsigned long long)target_bytes,
           (double)target_bytes / (1024 * 1024 * 1024));

    double start_time = get_time();

    // 创建子进程
    pid_t *pids = malloc(num_processes * sizeof(pid_t));
    if (!pids) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            free(pids);
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // 子进程执行基准测试
            int ret = benchmark_worker(target_bytes);
            free(pids);
            exit(ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
        } else {
            pids[i] = pid;
        }
    }

    // 父进程等待所有子进程完成
    int status;
    for (int i = 0; i < num_processes; i++) {
        waitpid(pids[i], &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
            fprintf(stderr, "子进程 %d 异常退出\n", pids[i]);
        }
    }

    double end_time = get_time();
    double elapsed = end_time - start_time;

    printf("所有进程完成，总耗时: %.3f 秒\n", elapsed);
    printf("总访问量: %llu 字节（%.2f GB）\n",
           (unsigned long long)(target_bytes * num_processes),
           (double)(target_bytes * num_processes) / (1024 * 1024 * 1024));
    printf("总吞吐量: %.2f MB/s\n",
           (double)(target_bytes * num_processes) / (1024 * 1024) / elapsed);

    free(pids);
    return 0;
}
