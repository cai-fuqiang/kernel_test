#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shm.h"
unsigned long get_chg_time = 0;

unsigned long *global_lock = NULL;

unsigned long *exec_proc = NULL;
int get_lock()
{
	__asm__(
#ifndef XCHG_BEG
	"Spin_Lock: \n"
	"cmpq $0, %[global_lock] \n"
	"je  Get_Lock \n"
	FILL_INST "\n"
	"jmp Spin_Lock \n"
	"Get_Lock: \n"
#endif
#ifdef XCHG_BEG
        "Spin_Lock: \n"
#endif
	"mov $1, %%rax \n"
	"xchg %%rax, %[global_lock] \n" //, %[locked_val] \n"
	"cmp $0, %%rax\n"
#ifdef XCHG_BEG
	FILL_INST "\n"
#endif
	"jne Spin_Lock \n"
	"Get_Lock_Success:"
	:
	[global_lock] "+m" (*global_lock)
	:
	:
	"%rax", "memory"
	);
	return 0;
} 

int release_lock()
{
	__asm__("movq $0, %[global_lock]":
			[global_lock] "+m" (*global_lock)
			:
			:);
}

int main()
{
	int i = 0;

	init_shm();
	global_lock = (unsigned long *)get_shm();
	*global_lock = 0;

	exec_proc = global_lock + 1;

	*exec_proc = *exec_proc + 1;
	while (*exec_proc < NEED_EXEC_NUM) {

	}
	printf("exec lock \n");
	for (i = 0; i < LOOP_TIME; i++) {
		get_lock();
		release_lock();
	}

	printf("get lock time %d\n", LOOP_TIME);

	exec_proc = 0;
}
