qemu-system-x86_64 -name guest=centos8,debug-threads=on \
	-m 2G -realtime mlock=off \
	-boot strict=on \
	-device virtio-scsi-pci,id=scsi0,bus=pci.0,addr=0x7 \
	-drive file=/root/workspace/centos_qcow2/CentOS-8-ec2-8.1.1911-20200113.3.x86_64.qcow2,if=none,id=drive-scsi0-0-0-0 \
	-device scsi-hd,bus=scsi0.0,channel=0,scsi-id=0,lun=0,drive=drive-scsi0-0-0-0,id=scsi0-0-0-0,bootindex=1 \
	-sandbox on,obsolete=deny,elevateprivileges=deny,spawn=deny,resourcecontrol=deny -msg timestamp=on -monitor stdio  -s -S
#-machine virt-rhel7.6.0,accel=kvm,usb=off,dump-guest-core=off,gic-version=3/-device pcie-root-port,bus=pcie.0,id=root_port \
	#-device pcie-root-port,port=0x8,chassis=1,id=pci.1,bus=pcie.0,multifunction=on,addr=0x1 \
	#-device pcie-root-port,port=0x9,chassis=2,id=pci.2,bus=pcie.0,addr=0x1.0x1 \
	#-no-user-config -nodefaults -rtc base=utc -no-shutdown \
	#-drive file=/root/workspace/centos_qcow2/OVMF/OVMF_CODE.secboot.fd,if=pflash,format=raw,unit=0,readonly=on \
	#-drive file=/root/workspace/centos_qcow2/OVMF/OVMF_VARS.secboot.fd,if=pflash,format=raw,unit=1 \
	#-serial telnet:localhost:88888,server,nowait \
