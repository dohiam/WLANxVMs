#!/bin/bash
## @file edu/install_targetfs.sh
## @brief builds edu and copies executables to the targetfs directory
## @details if there is any error then leave the error_file to indicate it
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).


# Standard cc build setup
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

touch $CLFS_ERROR_FILE
CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`
cd $CLFS_BUILD_DIR/$PACKAGE_NAME

# Build pci.ko and copy to targetfs
LOG="${CLFS_STATUS_DIR}/${PACKAGE_NAME}/logfile.txt"
touch $LOG
chmod ugo+rw $LOG
echo "compiling edu"
make CFLAGS="${CFLAGS}" KDIR="${CLFS_BUILD_DIR}/linux" ARCH="${CLFS_ARCH}" SYSROOT="$CLFS_SYSROOT_DIR" CROSS_COMPILE="${CLFS_TARGET}-"   > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting due to make failure"; exit $?; fi
echo "installing edu"
cp pci.ko ${CLFS_TARGETFS_DIR}/lib/modules   > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting due to make failure"; exit $?; fi

set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE