#!/bin/bash
dmesg -c > /dev/null
rmmod proctest
insmod proctest.ko
dmesg
