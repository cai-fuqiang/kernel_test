obj-m := pause_test.o

# 强制 -O0 优化级别
ccflags-y := -O0

# 如果编译环境需要指定内核路径，请设置 KDIR
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
