KVER            ?= /usr/src/kernels/$(shell uname -r)

BASEINCLUDE     ?= $(KVER)
proctest-objs   := livepatch_kvm_pmu.o
obj-m           := livepatch_kvm_pmu.o

all:
        $(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
        $(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
        rm -rf *.ko
