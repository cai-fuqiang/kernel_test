KVER		?= $(shell uname -r)

BASEINCLUDE 	?= /usr/src/kernels/$(KVER)
proctest-objs 	:= pvtlb_modify.o
obj-m			:= pvtlb_modify.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -rf *.ko
