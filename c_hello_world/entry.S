.code16
.text
.global mystart
mystart:
    ljmp $0, $.setcs
.setcs:
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $0x8c00, %esp
    mov $0xE57, %ax
    int $0x10
    mov $0xE0A, %ax
    int $0x10
    cld
    call main
# mov $__stack_top, %esp
