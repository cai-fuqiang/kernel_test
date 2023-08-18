#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "shm.h"
unsigned long get_chg_time = 0;

unsigned long *global_lock = NULL;

unsigned long *exec_proc = NULL;
int get_lock()
{
	int locked_val = 1;
	__asm__(
	"Spin_Lock: \n"
	"cmpq $0, (%[global_lock_addr]) \n"
	"je  Get_Lock \n"
	FILL_INST "\n"
	"jmp Spin_Lock \n"
	"Get_Lock: \n"
	"mov $1, %[locked_val] \n"
	"xchg %[locked_val], (%[global_lock_addr]) \n" //, %[locked_val] \n"
	"cmp $0, %[locked_val]\n"
	"jne Spin_Lock \n"
	"Get_Lock_Success:"
	::
	[global_lock_addr] "r" (global_lock),
	[locked_val] "r" (locked_val)
	:
	);
	return 0;
}
int release_lock()
{
	int unlock_val = 0;
	__asm("xchg %[unlock_val], (%[global_lock_addr])"::
			[global_lock_addr] "r" (global_lock),
			[unlock_val] "r" (unlock_val)
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
