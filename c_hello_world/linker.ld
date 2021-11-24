ENTRY(mystart)
SECTIONS
{
  . = 0x7c00;
  .stage1 :
  {
    entry.o(.stage1)
    *(.stage1)
  }
  __stage1_stack_bottom = .;
  . = . + 0x1000;
  __stage1_stack_top = .;
  /* https://stackoverflow.com/questions/53584666/why-does-gnu-ld-include-a-section-that-does-not-appear-in-the-linker-script */
  /*AT(ADDR(.stage1) + 512 - 2)*/
  . = 0x7dfe;
  .sig :
  {
      SHORT(0xaa55);
  }
  __stage2_text_beg = .;
  .stage2 : 
  {
    startup_32.o(.text)
    __gdt_table_base = .;
    startup_32.o(.gdt_table)
    main.o(.text)
    main.o(.data)
    main.o(.rodata)
    main.o(.bss)
    main.o(COMMON)
  }
  .text :
  {
    __start_32_entry = .;
    *(.start_32_session)
    *(.text)
    *(.data)
    *(.rodata)
    __bss_start = .;
    /* COMMON vs BSS: https://stackoverflow.com/questions/16835716/bss-vs-common-what-goes-where */
    *(.bss)
    __bss_end = .;
    *(COMMON)
  }
  __stage2_text_end = .;
  . = 0x8000;
  __stage2_stack_bottom = .;
  . = . + 0x2000;
  __stage2_stack_top = .;
  /* 
   * for img over 1M , if img disk not over  1M, ide read op may be false.
   */
  .end_data :
  {
    SHORT(0x0);
  }
  /DISCARD/ : {
    *(.eh_frame)
  }
}
