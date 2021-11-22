
void print_str(char *str, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        if (i != len) {
            __asm__ (
                "int $0x10" : : "a" ((0x0e << 8) | str[i])
            );
        } else {
            __asm__ (
                "int $0x10" : : "a" ((0x0e << 8) | '\0')
            );
        }
    }
    return ;
}

void main(void) {
    int i;
    int ip;
    char s_enter_main[] = "enter main .... \r\n";
    char s_hello_world[] = "hello world \r\n";

    print_str(s_enter_main, sizeof(s_enter_main) - 1);
    print_str(s_hello_world, sizeof(s_hello_world) - 1);

    while (1) {
        __asm__ ("hlt");
    };
}
