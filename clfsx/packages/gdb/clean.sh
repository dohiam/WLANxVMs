#!/bin/bash
## @file gdb/clean.sh
## @brief clean gdb files from the build directory
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# Standard cc build setup
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

PACKAGE="gdb"

rm -rf ${CLFS_BUILD_DIR}/$PACKAGE
rm -rf ${CLFS_BUILD_DIR}/${PACKAGE}-install
rm -rf ${CLFS_BUILD_DIR}/${PACKAGE}-build
rm -rf ${CLFS_STATUS_DIR}/$PACKAGE
rm -rf $CLFS_ERROR_FILE
