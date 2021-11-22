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
    mov $__stage1_stack_top, %esp

    cld
    /*
    mov $0xb800, %ax
    mov %ax, %es
    movb $'W', %es:0 
    movb $0x7c, %es:1
    movb $'A', %es:2
    movb $0x7c, %es:3
    */

	/*set read sector num */
	mov $1, %al
	mov $0x1f2, %dx 
	out %al, %dx

	/* set sector index */
	mov $0x1f3, %dx
	mov $1, %al
	out %al, %dx

	mov $0x1f4, %dx
	mov $0, %al
	out %al, %dx

	mov $0x1f5, %dx
	out %al, %dx

	mov $0x1f6, %dx
	mov $0xe0, %al
	out %al, %dx
	/* set read  area */
	/* send read cmd */
	mov $0x1f7, %dx
	mov $0x20, %al
	out %al, %dx

	mov $0, %bx
.waits:
	/* at this ,well need wait a moment, see %bx */
	inc %bx
	in %dx, %ax
	andb $0x88, %al
	cmpb $0x8, %al
	jnz .waits

	/* read disk data to memory */
	mov $0x1f0, %dx

	mov $__stage2_text_beg, %bx
	mov %bx, %cx
	add $512, %cx
.read_next:
	in %dx, %ax
	movw %ax, (%bx)
	add $2, %bx
	cmpw %cx, %bx
	jnz .read_next

.read_done:
	add $0, %bx
	jmp $0, $.read_done
