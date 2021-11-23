.code 32
.section data
gdt:
    .word gdt_end - gdt
    .long gdt
    /* eight-byte integer */
    /* see https://www.sourceware.org/binutils/docs-2.12/as.info/Index.html#Index */
    /*
     * low 32
     * [0~16]   {1}     segment limit 15:00
     * [16-31]  {0}     base address 15:00
     * ------------------------------------
     * high 32
     * [0-7]    {0}     base address 23:16
     * [8-11]   (1010)  type:code Execute/Read
     * [12]     1       desciptor type (0=system; 1 = code or data)
     * [13-14]  0       DPL
     * [15]     1       Segment present
     * [16-19]  {1}     Segment Limit 19:16
     * [20]     0       Available for use by system software
     * [21]     0       64-bit code segment (IA-32e mode only)
     * [22]     1       default operation size (0 = 16 bit segment; 1 = 32-bit segment)
     * [23]     1       Granularity
     * [24-31]  {0}     base 31:24
     */
    .quad 0x00cf9a000000ffff        /* KERNEL 32 CS */
    /*
     * high 32 
     * [21]     1       64-bit code segment (IA-32e mode only)
     * [22]     0       If L-bit is set, then D-bit must be cleared. When not in IA-32e mode
                        or for non-code segments, bit 21 is reserved and should always be set 
                        to 0.
     */
    .quad 0x00af9a000000ffff        /* KERNEL CS */
    /*
     * [8-11]   (0010)      type:data read/write
     */
    .quad 0x00cf92000000ffff        /* KERNEL DS */
    .quad   0x0080890000000000  /* TS descriptor */
    .quad   0x0000000000000000  /* TS continued */
gdt_end:

