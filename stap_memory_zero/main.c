#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
static int b = 0;
#define ALIGN_MALLOC
#ifdef ALIGN_MALLOC

#define my_malloc(align, size)	\
	aligned_alloc(align,size);
#else
#define my_malloc(algin, size)	\
	malloc(size,size);
#endif

#define PAGE_SIZE  (64 * 1024)
int is_all_zero(char *buf, size_t size)
{
	int i = 0;
	char tmp;
	for (i = 0; i < size; i++) {
		tmp = buf[i];
	}
}

int start_alloc()
{
        int size = 0;
        char *addr;
        int i = 0;
        int j = 0;
	char a = 0;
        for (i = 0; i < 4; i++) {
		b = 0;
                addr = my_malloc(PAGE_SIZE, PAGE_SIZE);
		if (!is_all_zero(addr, PAGE_SIZE)) {
			printf("my malloc allocate NOT ZERO memory\n");
		}
		if ( i != 3 ) {
			memset(addr, 0, PAGE_SIZE);
		}
		b = 0;
        }
	return 0;
}

int main()
{
	start_alloc();
	sleep(200);
        return 0;
}

                //addr = aligned_alloc(4096, 4096);
