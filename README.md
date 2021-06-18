@brief The intent of this project is to provide a way to test Wi-Fi implementations across multiple virtual machines.
@details
Overview
========
Testing across multiple virtual machines enables one to put an AP/router in one virtual machine (e.g., OpenWRT) and one or more STAs each in their own virtual machines,
creating a more realistic simulation than having them all running under the same OS in one virtual machine. It also allows disparate clients/AP which do not run under
the same OS to be tested. For example, an OpenWRT AP/router could be tested with an embedded Linux (e.g., Raspberry Pi) client and an Ubuntu web server. 

The approach used in this project is specific to Linux and Qemu/KVM:
1. Capture all packets from the hwsim0 adapter (from a Linux guest under Qemu/KVM),
2. Forward them to a server running in the Linux host machine which in turn sends them to all the clients connected to it,
3. Receive packets from the server into a different VM and inject them into the hwsim0 adapater in that guest.

This approach creates a virtual shared wireless medium between the virtual machines, albeit one that runs much more slowly that real Wi-Fi hardware. 
Capturing, sending, and then injecting packets adds some significant overhead and delay. The key steps that create some lag time are:
+ Copying packets from kernel space to user space and injecting them back into kernel space: the mechanisms for communication between VMs (sockets, vsock, shared memory) involve a driver 
that can be used by user space programs but not directly by other kernel drivers, so to get things out of one VM and into another, they need to go from kernel to user space first.
+ Copying packets from VM to host and from host to VM using sockets or vsock: the use of new shared memory might help; this has not yet been explored but it appears the shared memory exists on the host and
so some copying from VM to host and host to VM may still be involved. 
+ Both capture and injection methods must send the packets through the full network and driver stack to ensure full functonality is preserved in the simulaton. Any "short circuiting"
optimization must not short circuit any functionality.

Current Status
==============
The current level of maturity is best described as "proof of concept". It is sufficient to connect a client to a wireless AP and ping the gateway, so it has basic functionality. It also has
some debugging and profiling capability built in to enable exploring of improved approaches to moving packets between VMs.


Summary of the pieces needed to enable this approach
====================================================
1. A client running in each VM to capture, send, receive, and inject packets. This is implemented as the cross_injector application in this project.
2. A server running in the host to receive and broadcast all packets received. This is implemented as the bcaster application in this project. This application also displays a summary of packets 
(e.g., beacons, association, probe requests & responses, etc. In addition to displaying this high level summary in real time, it also recordes all packets seen in a pcap file so they can be analyzed with Wireshark.
3. Changes to the mac80211_hwsim driver to support packet injection and to allow a MAC address range difference between VMs in order to avoid conflicts (since different mac80211_hwsim instances 
are now talking to each other). This project provides a patch file to make these changes.
4. Changes to lengthen various timeouts to accomodate increased packet latency. This project provides a patch file to make these changes.
5. Some running Linux-based system that can be loaded and run in a VM, preferrably a really tiny system that takes little memory and loads very quickly, something typical of an embedded system. This is implemented in this
project as a set of bash scripts to download, build, and create a very basic installable image that all runs out of a initramfs. 
6. A build environment of some kind. In this project, this is provided by crosstools-ng. There is also a simple script and configuration file provided to do a one-click creation of all the tools.
7. A host system. The latest Ubuntu 20.04 was used. There are also several necessary support packages but there are scripts provided to get all the necessary packages to get this project up and running
on a fresh 20.04 install.
8. An environment to display and allow test scripts to issue coordinated commands to different terminal windows (i.e., different VMs, the host application, etc.). In this project this capability is provided
by tmux and some scripts that create and send commands to tmux windows. 
9. Preferrably some documentation too! :-)  This project uses doxygen to create documentation, e.g. this file.


Quckstart
=========
The following comands executed under a terminal prompt should enable you get all the necessary software, build it all, and see it work in a basic x86_64 environment, starting from a fresh Ubuntu 20.04 desktop install:
1. Install Ubuntu 20.4 desktop
2. untar xc.gz (or clone repository) so that you have the xc directory in some folder 
3. open a terminal and cd to the above created xc folder
3. cd xc/crosstools-ng
4. ./doitall.sh (or execute individual steps as ./install_prerequisites.sh, ./download.sh, ./build.sh 
5. cd ..
6. source env-vars.sh
7. cd x86-qemu
8. ./install_prerequisites.sh
9. ./build.sh
10. cd tools/bcaster
11. ./build.sh
12. cd ../../custom_packages/cross_injector/
13. ./build.sh
14. cd ../..
15. ./build.sh packages-ci
16. ./start_vsock.sh
17. ./tuntap.sh up
18. start new terminal window and maximize
19. cd to xc directory
20. source env-vars.sh
21. cd x86-qemu
22. ./start_tmux.sh
23. switch back to original terminal window
24. ./test_ping.sh
25. switch back to new terminal window and watch
    * The AP running in one VM in the top left window
    * The STA running in a second VM in the bottowm left window
    * The host program which receives packets and sends them to the other VM(s) in the top right window
    * A script running in the bottom right window that telents into the STA, brings up the wireless interface (you can ping 172.16.16.1 (the gateway) or enter other commands to explore/test functionality.
    * Note: if the STA interface does not automatically come up, try repeating the ./start_st1.sh script. Sometimes the it stops after the probe response without continuing to the association steps, but if you just do it again then it works.
    Once you see the wlan0 adapter up & running and associated with the AP then you can issue other commands to test things out, e.g., the ping_gw.sh script to ping the other VM. A screenshot of what you should see is in screenshot.png in the root directory of this project. There is even a video of this running on
a core 2 duo laptop in sample.mkv of the root directory. Install the vlc player on Ubuntu 20.04 to play this video.
26. To close everything down, assuming you are in the bottom right window under tmux:
    * type ctrl-b then up arrow to go to the tmux window running bcaster and issue ctrl-c to stop bcaster
    * type ctrl-b then left arrow to go to the tmux window running the VM with the AP and type ctrl-a and then x to kill this VM
    * type ctrl-b then down arrow to go to the tmux window running the VM with the STA and type ctrl-a and then x to kill this VM
    * now you can close the terminal window with all 4 tmux windows running it it, now that everything under these has been stopped
27. Optionally you can load bcaster.cap into Wireshark after ctrl-c in the upper right window to close bcaster. 
This file was created in the x86-qemu folder by bcaster and contails a pcap file of all the packets sent between the VMs.


To Build Documentation
======================
The following should enable you to see it work in a basic x86_64 environment:
1. cd to xc directory
2. ./build_docs.sh

Current Design
==============
There are several components/changes involved:
* The main component is called "cross injector" (CI) and is simply a user mode application that runs in each VM. It is what captures, injects, and communicates with the host server.
This application is in the x86-qemu/custom_packages/cross_injector folder.
* A host application that each VM's CI client. It receives packets and forwards them to all other VM's connected to it.
This application is in the x86-qemu/tools/bcaster folder.
* A patch to mac80211_hwsim that adds packet injection capability.
This patch is in the clfsx/patches folder.
* A patch to mac80211 that lengthens some of the timers that would otherwise trip due to the increased delays in sending packets in between VMs.
This patch is in the clfsx/patches folder.
* A small linux build system that builds Linux along with supporting packages sufficient to run, monitor, and debug the simulation.

Dependencies
============
* The current implementation is tied to mac80211_hwsim and thus to Linux. 
+ It uses libpcap to capture and inject.
+ It uses vsock to share packets with the host (and thus between VMs)

On advantage of this approach is that user mode applications see and possibly control every Wi-Fi packet, except for acks. Acks are currently handled directly in mac80211_hwsim. 
However, by combining this project with wmediumd, even this can be controlled so that one can drop packets at a low level too ([https://github.com/bcopeland/wmediumd]). 

Cross Compilation
-----------------
This is intended to simulate embedded devices so a cross compilation approach is used. Crosstools-ng is used to get the compiler, build tools, and libraries. The specific
architecture environment variables needed come from an "architecture.sh" file. An x86_64 platform was used to test everything. There is a architecture.sh file with the
settings for this and also a config file to build Crosstools-ng executables for the x86_64 platform. 

Linux Build Overview
--------------------
Both Buildroot and OpenWRT were first used to create the embedded Linux executables to run inside each VM, but in an effort to create as mimimal a running system as possible,
a custom build system was created that is more like Cross Linux From Scratch. This custom approach is just some Bash scripts to build each package, including the Linux kernel, 
create a minimal file system, and put them all together in an initramfs. An initramfs is used because the file system is tiny, initramfs boots very quickly and no persistance 
across each VM execution is needed.

Although the following is an overview of the build system, the details are all hidden if you just execute build.sh in the instructions.

The build system is divided into two parts. The first part is generic; this is in the clfsx (cross linux from scratch using x-tools) subdirectory. The subdirectories in clfsx are:
* ***Packages:*** contains a subdirectory for each package included in the Linux executable, including the kernel itself. Each of these package subdirectories has a script to:
    * Download and unpack the package from the internet
    * Build the package and install it into the root FS
    * Clean the build system, i.e., remove the package from the build directory
    * Configure the other scripts with things like the package name & version, URL to retrieve it, and how to unpack it.
    * Run all the above scripts and any other scripts as needed; this is the "recipe" file.
* ***Sources:*** This is a place to put the downloaded package files. If a package file already exists in this subdirectory, it will be used instead of downloading it.
* ***Patches:*** This has a few patch files to mac80211, mac80211_hwsim, etc
* ***Scripts:*** This contains some common functionality that is used repeatedly, like downloading and unpacking files

The second part is specific to the platform being tested (e.g., x86_64). This is in the x86-qemu directory and contains:
* ***architecture.sh:*** contains the environment variable values for building this for this target platform
* ***config files:*** for customizing packages. Namely, there is a config file for the linux kernel (kernel.config), busyboxy (busybox.config), wpa supplicant (wpa_supplicant.config),
and for hostapd (hostapd.config).
* ***my_targetfs:*** contains things desired to be in the target filesystem, such as startup scripts, hostapd.conf, and the interfaces file to ensure the targets boots up as desired.
* ***custom_packages:*** which contains custom applications for the target environment. This is where cross_injector lives. Any other custom applications would go here too.
* ***custom_actions:*** contains additional scripts not part of clfsx but are needed to build the desired target.
* ***package file(s):*** which list the packages desired to be in the target. It can refer to packages in clfsx or custom_packages. 

There are two different build environments:
1. ***"minimal":*** for building in a "clean" environment where the only environment variables that exist are ones tha describe the target environment. The 
regular host environment variables are removed before adding the target environment variables.
2. ***"regular":*** which builds things in the regular host environment, not the minimal one described above.

A package file can be fed to the build.sh script which simply lists the packages to include in the target:
* a package name
* a package name followed by a space and a specific build step. This would run a <step.sh> file from the package subdirectory. Unless this is followed by a comma and the word regular,
it is run in a minimal build environment.
* CUSTOM followed by a package name and optionally a package step runs the specified build action & step from the customer_actions subdirectory (under a regular build environment).
* CUSTOMIN which does the same as CUSTOM except in the minimal build environment.

Cross Injector (CI) Overview
----------------------------
* Each ci client can use vsock or regular networking for communication. The default is to use vsock.
* Each ci client has two queues, one for capturing and sending packets, and another for receiving and ijecting packets. Each has a fixed size queue, both in terms of number
of packets that can be queued and the size of each packet. This eliminates the overhead of dynamic memory.
* There is a single process/thread class (called "looper") which all the queues and workers use to run a single function over and over, concurrently with other 
functions being run over and over.
* the rest of the source files provide supporting functions to the above.


Bcaster Overview
----------------
* This handles multiple connected CI clients and can receive things in chunks for each client, recognizing when it has received a complete packet which it will then send to all connected clients.
* It provides some simple printed decoding of packets at a high level to show the progress of what is happening. The printed output is handled on a separate thread so as not to add to
the latency and will print packet times along with a timestamp in a different color for each client to help distinguish the back and forth interactions.
* For complete packet analysis, this application also writes each packet to a pcap file (bcaster.cap) in the current directory (x86-qemu) which can then be loaded into Wireshark for analysis.

CodeBlocks is used as the IDE for CI and Bcaster, so a good way to browse and modify these applications is to load their .cbp file into Code Blocks, in addition
to browsing the documentation for files under \ref x86-qemu/custom_packages/cross_injector/src.

Note on Profiling Timing Measurements
=====================================
The cross injector clients and the host bcaster applications have a way to tag packets with the guest real time clock time and then use this to track max, min, and average time in between
the capturing of a packet and the injection of a packet in other virtual machines. Each cross injector client will display its view of these statistics when ctrl-c is used to stop them
running inside a guest qemu/kvm session. By default, kvm synchronizes the guest rtc with the host so theoretically each guest should have a common view of real time. However, it has been
observed that this can vary by tens of milliseconds. This is sometimes enough to create situations where one cross injector client sees the captured time of a packet to be in the future
(i.e., the time a packet was injected is before the time it was captured in another VM). In the case of two VMs, one will show shorter times than the other, perhaps even negative times.
To get the real time, just average these times together. 

You should see that the lag time, which represents the overhad of moving packets in between VMs is on the order of 10 milliseconds. The time required to send a 500 bit payload on a 50mps wireless
link is on the order of 10 micro seconds. The simuation is clearly a lot slower than physical WiFi, not enough to make testing painful, but is enough to trip low level timeouts. Several optimizations
have been applied in this project and others such as the use of shared memory may also reduce lag time further, but I think the inherent overhead in this approach will never approch the
speed of physical Wi-Fi. One could try changing the clock in the simulations to make real time appear much slower to the low level wireless driver (mac80211) but this project currently just
adjusts the tiemout values themselves. 

Note that acks are handled directly in mac80211_hwsim. Each packet is internally acked before the packet is even sent to the other VM. This means that ack timeouts are not a problem in 
this approach, but it also means that simulating dropped packets would require further work. 

Note on mac80211_hwsim changes to enable injection
==================================================
The official mac80211_hwsim has a TODO for packet injection. I am no mac80211 expert but I filled this out largely by copying sections of code from other parts of the driver, and testing & adjusting
until it worked. I hope this is somewhat better than a "hack" but it has not been reviewed by experts. A future TODO is to try to get this or equivalent functionality enabled in the 
official version.

fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

