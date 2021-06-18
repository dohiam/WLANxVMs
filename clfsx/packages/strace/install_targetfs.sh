#!/bin/bash
## @file strace/install_targetfs.sh
## @brief builds strace and copies executables to the targetfs directory
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

# Build strace
LOG="${CLFS_STATUS_DIR}/${PACKAGE_NAME}/logfile.txt"
touch $LOG
chmod ugo+rw $LOG
echo "configuring strace"
./configure --build="${CLFS_HOST}" --host="${CLFS_HOST}" --target="${CLFS_TARGET}" --prefix="${CLFS_BUILD_DIR}/strace-install"  > $LOG 2>&1
#./configure --build="${CLFS_HOST}" --host="${CLFS_HOST}" --target="${CLFS_TARGET}" --prefix="${CLFS_BUILD_DIR}/strace-install"
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi
echo "compiling strace"
make -j $JOBS  > $LOG 2>&1
#make 
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi
echo "installing strace"
make install  > $LOG 2>&1
#make install 
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi
cp -r ${CLFS_BUILD_DIR}/strace-install/* ${CLFS_TARGETFS_DIR}

set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE