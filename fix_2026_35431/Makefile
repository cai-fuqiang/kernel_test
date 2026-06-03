ifneq ($(KERNELRELEASE),)

ifeq ($(VERSION).$(PATCHLEVEL),5.10)
obj-m += cve_31431_mitigate_5_10.o
cve_31431_mitigate_5_10-y := cve-31431-mitigate-5.10.o
else
obj-m += cve_31431_mitigate.o
cve_31431_mitigate-y := cve-31431-mitigate.o
endif

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
