

int print_ip(int ip)
{
	int tmp = 0;
	int i = 0;
	int j = 0;
	char v = 0;
	char buf[16] = {0};

	tmp = ip;

	buf[i++] = '\n';

	do {
		v = tmp % 10 + '0';
		buf[i] = v;
		tmp = tmp / 10;
		i++;
	}while(tmp != 0);
	j = i;
	while(i >= 0) {
        	__asm__ (
        	    "int $0x10" : : "a" ((0x0e << 8) | buf[i])
        	);
		i--;
	}

	return j;
}

void main(void) {
    int i;
    int ip;
    char s[] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};

    __asm__("call NEXT\n\t"
    "NEXT:\n\t"
    "pop %%ax"
    : "=a"(ip));

    i = print_ip(ip);
    print_ip(i);
    for (i = 0; i < sizeof(s); ++i) {
        __asm__ (
            "int $0x10" : : "a" ((0x0e << 8) | s[i])
        );
    }
    while (1) {
        __asm__ ("hlt");
    };
}
