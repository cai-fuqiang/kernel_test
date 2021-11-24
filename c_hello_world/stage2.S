.code16
.section ".stage2", "ax"
.stage2_beg:
    mov $0xb800, %ax
    mov %ax, %es
    movb $'W', %es:0 
    movb $0x8c, %es:1
    movb $'F', %es:2
    movb $0x8c, %es:3
    movb $'Q', %es:4
    movb $0x8c, %es:5
    mov $__stage2_stack_top, %esp
    cld
    call main
