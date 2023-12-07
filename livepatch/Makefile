KVER		?= /usr/src/kernels/$(shell uname -r)

BASEINCLUDE 	?= $(KVER)
proctest-objs 	:= livepatch-sample-2.o
obj-m			:= livepatch-sample-2.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -rf *.ko
