KVER ?= $(shell uname -r)
BASEINCLUDE 	?= /usr/src/kernels/$(KVER)
proctest-objs 	:= spinlocktest.o
obj-m			:= spinlock_test.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -rf *.ko
