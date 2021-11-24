#include "tools.h"

void main_32() __attribute((section(".start_32_session")));

void test2()
{

    __asm__ (
        "int $0x10" : : "a" ((0x0e << 8) | 'L')
    );

}
void main_32()
{
    static char enter_32[] = "enter 32 ?\n";
    int i = 0;
    for (i = 0; i < 10; i++) {
        __asm__ (
            "int $0x10" : : "a" ((0x0e << 8) | 'W')
        );
    }
    //print_str(enter_32, sizeof(enter_32) - 1);
    while (1) {
        __asm__ ("hlt");
    };
    return;
}
