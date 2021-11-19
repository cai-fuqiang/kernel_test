ENTRY(mystart)
SECTIONS
{
  . = 0x7c00;
  .stage1 :
  {
    entry.o(.stage1)
    *(.stage1)
  }
  /* https://stackoverflow.com/questions/53584666/why-does-gnu-ld-include-a-section-that-does-not-appear-in-the-linker-script */
  .sig : 
  /*AT(ADDR(.text) + 512 - 2)*/
  {
      SHORT(0xaa55);
  }
  .stage2 : 
  {
    stage2.o(.stage2)
    *(.stage2)
  }
  .text :
  {
    *(.text)
    *(.data)
    *(.rodata)
    __bss_start = .;
    /* COMMON vs BSS: https://stackoverflow.com/questions/16835716/bss-vs-common-what-goes-where */
    *(.bss)
    __bss_end = .;
    *(COMMON)
  }
  /DISCARD/ : {
    *(.eh_frame)
  }
  __stack_bottom = .;
  . = . + 0x1000;
  __stack_top = .;
}
