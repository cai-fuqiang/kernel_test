BASEINCLUDE 	?= /usr/src/kernels/$(shell uname -r)
proctest-objs 	:= proc-test.o
obj-m			:= proctest.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -rf *.ko
