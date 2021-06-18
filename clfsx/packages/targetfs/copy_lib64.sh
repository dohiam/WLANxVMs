#!/bin/bash
## @file targetfs/copy_lib64.sh
## @brief copies lib64 from the xtools directory to the target directory
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

. ${CLFSPROJ}/architecture.sh
. ${CLFS}/scripts/common.sh

CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`
cp -Lr ${XTOOLS}/${CLFS_HOST}/${CLFS_TARGET}/lib64/* ${CLFS_TARGETFS_DIR}/usr/local/lib
set_status $PACKAGE_NAME $STEP "completed_successfully"
