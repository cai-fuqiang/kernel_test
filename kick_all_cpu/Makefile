BASEINCLUDE 	?= /usr/src/kernels/4.18.0-240.1.1.el8_3.x86_64/
proctest-objs 	:= kick_all_cpu.o
obj-m			:= kick_all_cpu.o

all:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) modules;

clean:
	$(MAKE) -C $(BASEINCLUDE) M=$(PWD) clean;
	rm -rf *.ko
