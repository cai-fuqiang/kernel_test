#!/bin/sh
gdb --nx -ix conf/gdb.txt main.elf -ex 'target remote localhost:1234' -ex 'hb *0x7e00' -ex 'c'
