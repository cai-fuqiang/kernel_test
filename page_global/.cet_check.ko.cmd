savedcmd_/home/wang/workspace/kernel_test/page_global/cet_check.ko := ld -r -m elf_x86_64 -z noexecstack --no-warn-rwx-segments --build-id=sha1  -T scripts/module.lds -o /home/wang/workspace/kernel_test/page_global/cet_check.ko /home/wang/workspace/kernel_test/page_global/cet_check.o /home/wang/workspace/kernel_test/page_global/cet_check.mod.o;  make -f ./arch/x86/Makefile.postlink /home/wang/workspace/kernel_test/page_global/cet_check.ko
