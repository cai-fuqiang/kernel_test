#!/bin/sh
gdb --nx -ix conf/gdb.txt main.elf -ex 'target remote localhost:1234'
