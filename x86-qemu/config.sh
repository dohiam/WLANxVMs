#!/bin/bash
## @file x86-qemu/config.sh
## @brief x86 configuration file
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# USE_EXISTING_PACKAGE
#
# If true and package already exists, it will be used as is. If false then it will always be unzipped from source.

USE_EXISTING_PACKAGE=true

# USE_EXISTING_SOURCE
#
# If true and source already exists, it will be used as is. If false then it will always be downloaded from the original source.

USE_EXISTING_SOURCE=true

# NUMBER OF PARALLEL COMPILES

JOBS=2

# Initial CFLAGS

CFLAGS_DEFAULT=""
CFLAGS=" -I/home/david/xc/x-tools/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/sysroot/usr/include -I/home/david/xc/ramdisk/build/libpcap-install/include"

# Indicate if MUSL libraries are used

MUSL=false

# Cross Compile or build for host system

CROSS_COMPILE=false

# If using drivers as modules

USING_MODULES=true

