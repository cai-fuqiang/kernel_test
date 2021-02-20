BASEINCLUDE 	?= /usr/src/kernels/4.18.0-240.1.1.el8_3.x86_64/
proctest-objs 	:= proc-test.o
obj-m			:= proctest.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -rf *.ko
