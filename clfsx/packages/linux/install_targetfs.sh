#!/bin/bash
## @file linux/install_targetfs.sh
## @brief builds linux and copies executables to the targetfs directory
## @details if there is any error then leave the error_file to indicate it
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# if argument passed then recipe is run for a specific purpose so skip out
# running in minimal shell gloms args together so split back out
args=($1)
if [[ ${#args[@]} -gt 0 ]]; then exit 0; fi

# Standard cc build setup
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

touch $CLFS_ERROR_FILE

CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`
cd $CLFS_BUILD_DIR/$PACKAGE_NAME
# Build linux to run on target
LOG="${CLFS_STATUS_DIR}/${PACKAGE_NAME}/logfile.txt"
touch $LOG
chmod ugo+rw $LOG
echo "preparing to build linux"
make mrproper > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure at make proper"; exit $?; fi
echo "using existing kernel.config"
#cp ${CLFSPROJ}/kernel.config .config
cp ${CLFSPROJ}/kernel.config .config
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi
echo "compiling kernel"
make -j $JOBS ARCH="${CLFS_ARCH}" SYSROOT="$CLFS_SYSROOT_DIR" CROSS_COMPILE="${CLFS_TARGET}-" > $LOG 2>&1 
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure at at initial make"; exit $?; fi
echo "compiling modules"
make -j $JOBS ARCH="${CLFS_ARCH}" CROSS_COMPILE="${CLFS_TARGET}-" INSTALL_MOD_PATH="${CLFS_TARGETFS_DIR}" modules_install > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure at modules_install"; exit $?; fi
#make -j $JOBS ARCH=${CLFS_ARCH} CROSS_COMPILE=${CLFS_TARGET}- INSTALL_MOD_PATH=${CLFS_TARGETFS_DIR} modules_prepare
#if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure at modules_prepare"; exit $?; fi
#make -j $JOBS ARCH=${CLFS_ARCH} CROSS_COMPILE=${CLFS_TARGET}-
#if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure at at final make"; exit $?; fi
set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE