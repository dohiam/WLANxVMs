#!/bin/bash

set +x
sudo apt-get update
sudo apt-get install libpcap-dev bison codeblocks tmux qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils virtinst virt-manager -y
