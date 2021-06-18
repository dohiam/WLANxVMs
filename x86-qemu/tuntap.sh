#!/bin/bash

#set -x

if [ $# -lt 1 ]
then
    echo "Usage: $0 [up | down]"
    exit 1
fi

#######################################################################################
#                                                            
#                                   localhost                                   
#                                       |                                   
#                                       |                                   
#    /----------------------------------o--------------------------------------\             
#    |                   |          qemu NAT           |                       |             
#    \--------o----------|--------------o--------------|----------o------------/           
#             |                         |                         |
#             |10.0.2.11                |10.0.2.12                |10.0.2.13
#             |                         |                         |
#     /-------o-------\         /-------o-------\         /-------o-------\                                                  
#    |       wan       |       |       wan       |       |       wan       |                                   
#    |                 |       |                 |       |                 |                                   
#    |      (VM1)      |       |      (VM1)      |       |      (VM1)      |                                   
#    |                 |       |                 |       |                 |                                   
#    |       lan       |       |       lan       |       |       lan       |                                   
#     \-------o-------/         \-------o-------/         \-------o-------/                               
#             |                         |                         |
#             |192.168.224.101          |192.168.224.102          |192.168.224.103
#             |(tap1)                   |(tap2)                   |(tap3)
#             |                         |                         |
#    /--------o-------------------------o-------------------------o-------------\             
#    |                                 br0                                      |             
#    \----------------------------------o---------------------------------------/           
#                                       |                      
#                                       |192.168.224.100                      
#                                       |(tap0)
#                                       |                      
#                                  host_network                         
#                                                            
#######################################################################################

HOST_IF="enp1s0"
USER="${USERNAME}"
GROUP="${USERNAME}"
TAP0="tap0"
TAP1="tap1"
TAP2="tap2"
TAP3="tap3"
IPRANGE="192.168.224.0/24"
IP0="192.168.224.100"
IP1="192.168.224.101"
IP2="192.168.224.102"
IP3="192.168.224.103"
BRIDGE="br0"

################################################################
if [ "$1" = "up" ]
then
    sudo ip tuntap add dev $TAP0 mode tap user $USER group $GROUP
    sudo ip tuntap add dev $TAP1 mode tap user $USER group $GROUP
    sudo ip tuntap add dev $TAP2 mode tap user $USER group $GROUP
    sudo ip tuntap add dev $TAP3 mode tap user $USER group $GROUP
    sudo ip link set dev $TAP0 up 
    sudo ip link set dev $TAP1 up
    sudo ip link set dev $TAP2 up
    sudo ip link set dev $TAP3 up
    #sudo ip addr add $IPRANGE dev $TAP0
	#sudo ifconfig $TAP0 $IP0 up
    #sudo ip addr add $IP1/24 dev $TAP1
    #sudo ip addr add $IP2/24 dev $TAP2
    #sudo ip addr add $IP3/24 dev $TAP3
    sudo ip link add name $BRIDGE type bridge
    sudo ip link set $BRIDGE up
    sudo ip link set $TAP0 master $BRIDGE
    sudo ip link set $TAP1 master $BRIDGE
    sudo ip link set $TAP2 master $BRIDGE
    sudo ip link set $TAP3 master $BRIDGE
elif [ "$1" = "down" ]
then
    sudo ip link set $TAP0 nomaster
    sudo ip link set $TAP1 nomaster
    sudo ip link set $TAP2 nomaster
    sudo ip link set $TAP3 nomaster
    sudo ip link set $BRIDGE down
    sudo ip link delete $BRIDGE type bridge
    sudo ip link set $TAP0 down
    sudo ip link set $TAP1 down
    sudo ip link set $TAP2 down
    sudo ip link set $TAP3 down
    sudo ip tuntap del dev $TAP0 mode tap
    sudo ip tuntap del dev $TAP1 mode tap
    sudo ip tuntap del dev $TAP2 mode tap
    sudo ip tuntap del dev $TAP3 mode tap
else
    echo "Usage: $0 [up | down]"
    exit 1
fi
