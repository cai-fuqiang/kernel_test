static void print_str(char *str, int len)
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
