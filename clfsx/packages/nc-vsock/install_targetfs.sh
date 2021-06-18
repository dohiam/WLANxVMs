#!/bin/bash
## @file nc-vsock/install_targetfs.sh
## @brief builds nc-vsock and copies executables to the targetfs directory
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

# Compile nc-sock and copy executable to targetfs
$CC nc-vsock.c -o nc-vsock
if [ $? -ne 0 ]; then echo "exiting due to make failure"; exit $?; fi
cp nc-vsock ${CLFS_TARGETFS_DIR}/bin
if [ $? -ne 0 ]; then echo "exiting due to make failure"; exit $?; fi

set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE