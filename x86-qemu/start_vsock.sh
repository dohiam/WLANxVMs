#!/bin/bash
## @file start_vsock.sh
## @brief vsock; execute this once per linux session (i.e., run this again after rebooting the host system)
#

sudo modprobe vhost_vsock
sudo chmod 0666 /dev/vhost-vsock
sudo usermod -a -G kvm $USERNAME
