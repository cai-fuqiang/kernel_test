ifneq ($(KERNELRELEASE),)

obj-m += cve_31431_mitigate.o
cve_31431_mitigate-y := cve-31431-mitigate.o

else

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

.PHONY: all modules clean

all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

endif
