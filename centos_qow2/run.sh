#!/usr/bin/env bash
COM=$1

if [ "$COM" == "mount" ];then
	qemu-nbd -c /dev/nbd0 ./CentOS-8-ec2-8.1.1911-20200113.3.x86_64.qcow2
	sleep 1
	mount /dev/nbd0p1 ./mnt
else
	umount ./mnt
	qemu-nbd -d /dev/nbd0
fi
