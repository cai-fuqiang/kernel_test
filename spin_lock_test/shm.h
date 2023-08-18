#ifndef __SHM_H__
#define __SHM_H__
#define OBJ_COUNT 	1024 
//#define SHM_SIZE	(OBJ_COUNT * sizeof(unsigned long))
#define SHM_SIZE	(1024 * 16)

#define LOOP_TIME	5000000
//#define MAX_LOOP_TIME	10000000000
//#define NEED_EXEC_NUM 8
#define __STR(x) 	#x
#define STR(x) 		__STR(x)
int init_shm();
char *get_shm();

#if BUILD_PAUSE == 1
#define FILL_INST "pause"
#else
#define FILL_INST "nop"
#endif

//#define NEED_LOCK 0
//
//#if NEED_LOCK == 0
//#define LOCK_PRE 	" "
//#else
//#define LOCK_PRE 	"lock;"
//#endif

#endif
