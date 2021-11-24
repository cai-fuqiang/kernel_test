#include "tools.h"
extern void main_32();

void main(void) {
    int i;
    int ip;
    char s_enter_main[] = "enter main .... \r\n";
    char s_hello_world[] = "hello world \r\n";

    print_str(s_enter_main, sizeof(s_enter_main) - 1);
    print_str(s_hello_world, sizeof(s_hello_world) - 1);

    __asm__ ("callw __start_32_entry\n\t");

    return ;
}
