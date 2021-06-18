@dir clfsx
@brief A Cross Linux from Scratch style build of a basic bootable Linux system with supporting utilities for wlan.

@dir clfsx/packages 
@brief Scripts to download and compile basic packages

@dir clfsx/packages/bootscripts
@brief Scripts to download and build bootscripts

@dir clfsx/packages/busybox
@brief Scripts to download and build busybox

@dir clfsx/packages/clfs_system
@brief Scripts to set up the top level project directories

@dir clfsx/packages/CUSTOM
@brief Scripts to redirect back to the target specific packages (using normal host environment)

@dir clfsx/packages/CUSTOMIN
@brief Scripts to redirect back to the target specific packages (using minimal target-only environment)

@dir clfsx/packages/edu
@brief Scripts to download and build the edu package (an example qemu driver, for educational purposes)

@dir clfsx/packages/gdb
@brief Scripts to download and build gdb (Gnu debugger, for debugging in the target environment)

@dir clfsx/packages/hostapd
@brief Scripts to download and build hostapd (to run the Wi-Fi AP)

@dir clfsx/packages/iana-etc
@brief Scripts to download and build iana-etc (iana port assignments)

@dir clfsx/packages/iw
@brief Scripts to download and build iw (iwconfig)

@dir clfsx/packages/libconfig
@brief Scripts to download and build libconfig (library for processing configuration files, used by wmediumd)

@dir clfsx/packages/libnl
@brief Scripts to download and build libnl (library for interfacing with netlink - communications mechanism between drivers and user mode)

@dir clfsx/packages/libpcap
@brief Scripts to download and build libpcap (library for packet capture and injection)

@dir clfsx/packages/linux
@brief Scripts to download and build the linux kernel 

@dir clfsx/packages/nc-vsock
@brief Scripts to download and build nc-vsock (an example of using vsock, to mimic the netcat utility over netlink)

@dir clfsx/packages/openssl
@brief Scripts to download and build openssl (pre-req for WPA supplicant)

@dir clfsx/packages/strace
@brief Scripts to download and build strace (utility to monitor linux system calls)

@dir clfsx/packages/targetfs
@brief Scripts download and build the target file system

@dir clfsx/packages/tcpdump
@brief Scripts to download and build the tcpdump utility (to capture and display packages independently of cross_injector, for debugging)

@dir clfsx/packages/wireless_tools
@brief Scripts to download and build wireless tools (for configuring and status checking wlan interfaces)

@dir clfsx/packages/wpa_supplicant
@brief Scripts to download and build the wpa supplicant (for wlan authentication)

@dir clfsx/packages/wireless_tools
@brief Scripts to download and build wireless tools (for configuring and status checking wlan interfaces)

@dir clfsx/scripts
@brief Common download and build scripts (used by scripts in the packiages directory)

@dir x86-qemu
@brief Target and application specific pieces (namely cross_injector and configuration for x86_64)

@dir x86-qemu/custom_actions
@brief Custom actions including scripts to copy library files from cross-tools and to build/include custom applications (like cross_injector)

@dir x86-qemu/custom_packages
@brief Applications like cross_injector and wmediumd

@dir x86-qemu/custom_packages/cross_injector
@brief Source files for the cross_injector application and configuration to build and maintain under code blocks

@dir x86-qemu/my_targetfs
@brief Files to be included in the target filesystem (initramfs)

@dir x86-qemu/tools
@brief Applications that run on the host, not the target (like bcaster, the server that cross_injector clients connect to in order to share packets between VMs)








