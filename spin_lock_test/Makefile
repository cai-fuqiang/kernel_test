EXEC_OBJ 	:= spinlock_pause spinlock_nopause clean_exec_num
cflags		:= -DNEED_EXEC_NUM=$(NEED_EXEC_NUM)

ifeq ($(XCHG_BEG), 1)
cflags		+= -DCHG_BEG
endif

all: $(EXEC_OBJ)

LIB_O	:= shm.o

%.o:%.c
	gcc $(cflags) -O0 -c $< -o $@ -g -DBUILD_PAUSE=1

spinlock_pause:spin_lock.o $(LIB_O)
	gcc -o $@ $^

spinlock_nopause:spin_lock.c $(LIB_O)
	gcc -O0 $(cflags) -g -DBUILD_PAUSE=0 -o $@ spin_lock.c $(LIB_O)
clean_exec_num:clean_exec_num.o $(LIB_O)
	gcc -o $@ $^
clean:
	rm -f *.o  $(EXEC_OBJ) *.S *.I
#no_pause:no_pause.o $(LIB_O)
#	gcc -o $@ $^ 
