#include "tools.h"

void main_32() __attribute((section(".start_32_session")));

void test2()
{
	//char *ptr = 0xb8000;
	//*ptr = ' ';
	//ptr++;
    	__asm__ (
	    "movb $0, %eax\n\t"
    	    "movb $' ', 0xb8000(%eax)\n\t"
    	    "movb $0x8c, (0xb8001)\n\t"
    	    "movb $'H', (0xb8002)\n\t"
    	    "movb $0x8c, (0xb8003)\n\t"
    	);
	//*ptr = 0x8c;
	//ptr++;
	//*ptr = 'H';
	//ptr++;
	//*ptr++ = 0x8c;
	return ;
}
void main_32()
{
    static char enter_32[] = "enter 32 ?\n";
    int i = 0;
    //for (i = 0; i < 10; i++) {
    //    __asm__ (
    //        "int $0x10" : : "a" ((0x0e << 8) | 'W')
    //    );
    //}
    //print_str(enter_32, sizeof(enter_32) - 1);
    test2();
    while (1) {
        __asm__ ("hlt");
    };
    return;
}
