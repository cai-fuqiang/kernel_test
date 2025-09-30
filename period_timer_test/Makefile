obj-m += periodic_hrtimer.o
KDIR := /export/wfq/kernel/linux-6.16.9

all:
	    make -C $(KDIR) M=$(PWD) modules
clean:
	    make -C $(KDIR) M=$(PWD) clean
