.code16
.section ".stage1", "ax"
.global mystart
mystart:
    ljmp $0, $.setcs
.setcs:
    xor %ax, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov $__stack_top, %esp
    mov $0xb800, %ax
    mov %ax, %es
    movb $'W', %es:0 
    movb $0x7c, %es:1
    cld
    call main
