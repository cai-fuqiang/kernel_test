.code16
.section ".stage2", "ax"
.stage2_beg:
    mov $0xb800, %ax
    mov %ax, %es
    movb $'W', %es:0 
    movb $0x8c, %es:1
    cld
    call main
