#!/bin/bash
## @file custom_actions/bear_linux.sh
## @brief special build to create build ear (bear) for use in sourcetrail
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).


cd /home/david/xc/ramdisk/build/linux

# Standard cc build setup
. $CLFSPROJ/config
. $CLFSPROJ/architecture
. $CLFS/scripts/common
touch $CLFS_ERROR_FILE

CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`

# Build linux to run on target
make mrproper
cp ${CLFSPROJ}/kernel.config .config
bear -o /home/david/linux/bear/linux.cdb make ARCH="${CLFS_ARCH}" SYSROOT="$CLFS_SYSROOT_DIR" CROSS_COMPILE="${CLFS_TARGET}-"

