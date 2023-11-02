KVER		?= $(shell uname -r)

BASEINCLUDE 	?= /usr/src/kernels/$(KVER)
proctest-objs 	:= cet_check.o
obj-m			:= cet_check.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -rf *.ko
