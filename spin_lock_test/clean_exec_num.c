#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shm.h"
unsigned int *exec_proc = NULL;

unsigned int *global_lock = NULL;
int main()
{
	int i = 0;

	init_shm();
	global_lock = (unsigned int *)get_shm();
	if (!global_lock) {
		printf("get shm mem error\n");
		return 1;
	}

	exec_proc = global_lock + 1;
	*exec_proc = 0;

	return 0;
}
