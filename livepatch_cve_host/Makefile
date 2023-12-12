KVER		?= /usr/src/kernels/$(shell uname -r)

BASEINCLUDE 	?= $(KVER)
proctest-objs 	:= livepatch_steal_time.o
obj-m			:= livepatch_steal_time.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -rf *.ko
