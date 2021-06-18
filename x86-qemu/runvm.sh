#!/bin/bash
## @file runvm.sh
## @brief this starts a virtual machine with the build linux kernel and initramfs with useful ports for communicating with the host and vsock
#

set -x

if [ $# -lt 1 ]
then
    TUNTAP=false
	NUM=1
else
    TUNTAP=true
	NUM=$1
fi

################################################################

IMAGE="./bzImage"


QEMU="kvm"
MEM="512M" 
CPU="host"
#OTHER_DEVICES="-device edu"
OTHER_DEVICES=""
LAN_DEVICE="e1000"
WAN_DEVICE="e1000"
#MACHINE="virt"
MACHINE="q35"

LAN='eth0'
WAN='eth1'

b=$((${a}+1))
CID=$((${NUM}+2))
TAP="tap${NUM}"
SSH_PORT="555${NUM}"
TEL_PORT="666${NUM}"
GDB_PORT="777${NUM}"
FTP_PORT="888${NUM}"
LAN_MAC_ADDR="52:54:00:12:34:5${NUM}"
WAN_MAC_ADDR="52:54:00:12:34:6${NUM}"
WAN_NAT_ADDR="10.0.2.1${NUM}"
NAT_RANGE="10.0.2.0/24"

net=192.168.76.0/24,dhcpstart=192.168.76.9

################################################################

# -s -S

if ($TUNTAP); then
    $QEMU -m $MEM $OTHER_DEVICES                                                                                      \
	  -netdev tap,id=$LAN,ifname=$TAP,script=no,downscript=no                                                            \
	  -device $LAN_DEVICE,netdev=$LAN,mac=$LAN_MAC_ADDR                                                                  \
	  -netdev user,id=$WAN,net=$NAT_RANGE,dhcpstart=$WAN_NAT_ADDR,hostfwd=tcp::$FTP_PORT-:21,hostfwd=tcp::$TEL_PORT-:23,hostfwd=tcp::$GDB_PORT-:2159   \
	  -device $WAN_DEVICE,netdev=$WAN,mac=$WAN_MAC_ADDR                                                                  \
	  -device vhost-vsock-pci,id=vhost-vsock-pci0,guest-cid=${CID}                                                       \
	  -cpu $CPU -M $MACHINE -nographic -m 128 -kernel $IMAGE                       										 \
	  -append "earlyprintk=serial,ttyS0 console=ttyS0 debug init=/sbin/init"
else
    $QEMU -m $MEM $OTHER_DEVICES                                                                                      \
	  -netdev user,id=$LAN,net=192.168.76.0/24,dhcpstart=192.168.76.9                                                    \
	  -device $LAN_DEVICE,netdev=$LAN,mac=$LAN_MAC_ADDR                                                                  \
	  -netdev user,id=$WAN,net=$NAT_RANGE,dhcpstart=$WAN_NAT_ADDR,hostfwd=tcp::$FTP_PORT-:21,hostfwd=tcp::$TEL_PORT-:23,hostfwd=tcp::$GDB_PORT-:2159   \
	  -device $WAN_DEVICE,netdev=$WAN,mac=$WAN_MAC_ADDR                                                                  \
	  -device vhost-vsock-pci,id=vhost-vsock-pci0,guest-cid=${CID}                                                       \
	  -cpu $CPU -M $MACHINE -nographic -m 128 -kernel $IMAGE                                                             \
	  -append "earlyprintk=serial,ttyS0 console=ttyS0 debug init=/sbin/init"
fi

