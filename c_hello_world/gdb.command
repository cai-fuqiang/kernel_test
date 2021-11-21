#!/bin/sh
gdb --nx -ix gdb.txt main.elf -ex 'target remote localhost:1234'
