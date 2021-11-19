.code16
.section ".stage2", "ax"
.stage222:
    mov $0xb800, %ax
    mov %ax, %es
    movb $'S', %es:0 
    movb $0x8c, %es:1
    cld
    call main
