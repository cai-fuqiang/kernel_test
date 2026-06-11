KDIR ?= /lib/modules/$(shell uname -r)/build

obj-m := ipi_latency.o

all: module userspace

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

userspace: ipi_latency_test.c
	$(CC) -O2 -Wall -o ipi_latency_test ipi_latency_test.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f ipi_latency_test
