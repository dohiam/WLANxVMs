#!/bin/bash
## @file libnl/install_targetfs.sh
## @brief builds libnl and copies executables to the targetfs directory
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

# Build libnl
# Install to build directory where tcpdump can find it and copy to targetfs
LOG="${CLFS_STATUS_DIR}/${PACKAGE_NAME}/logfile.txt"
touch $LOG
chmod ugo+rw $LOG
echo "configuraing libnl"
CC="${CC} -g "
export CC
./configure --build=${CLFS_HOST} --host=${CLFS_TARGET} --with-sysroot=${CLFS_SYSROOT_DIR} --prefix=${CLFS_BUILD_DIR}/libnl-install   > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi
echo "compiling libnl"
make -j $JOBS  > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi
echo "installing libnl"
make install  > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi

cp -r ${CLFS_BUILD_DIR}/libnl-install/* ${CLFS_TARGETFS_DIR}

set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE