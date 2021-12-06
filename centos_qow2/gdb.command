#!/bin/sh
gdb  -ix conf/gdb.txt  -ex 'target remote localhost:1234'

