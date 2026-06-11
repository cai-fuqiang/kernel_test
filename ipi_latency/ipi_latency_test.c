// SPDX-License-Identifier: GPL-2.0
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
		if (opt == 'n') {
			n = atoi(optarg);
			if (n <= 0) {
				fprintf(stderr, "samples must be > 0\n");
				return 1;
			}
		} else {
			fprintf(stderr, "Usage: %s [-n samples]\n", argv[0]);
			return 1;
		}
	}

	int ncpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
	if (ncpus <= 0) {
		fprintf(stderr, "sysconf(_SC_NPROCESSORS_ONLN) failed\n");
		return 1;
	}

	unsigned long long (*avg)[ncpus] = calloc(ncpus, sizeof(*avg));
	if (!avg) {
		perror("calloc");
		return 1;
	}

	printf("Measuring IPI latency (%d CPUs, %d samples per pair)...\n",
	       ncpus, n);

	for (int src = 0; src < ncpus; src++) {
		for (int dst = 0; dst < ncpus; dst++) {
			unsigned long long mn, mx, av;
			if (src == dst)
				continue;
			if (measure(src, dst, n, &mn, &mx, &av) < 0) {
				fprintf(stderr, "FAIL cpu%d->cpu%d\n", src, dst);
				free(avg);
				return 1;
			}
			avg[src][dst] = av;
		}
	}

	printf("\nIPI Latency Matrix - avg (ns) [%d samples]\n\n", n);
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

	free(avg);
	return 0;
}
