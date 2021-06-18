#!/bin/bash
## @file iw/clean.sh
## @brief clean iw files from the build directory
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# Standard cc build setup
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

PACKAGE="wireless_tools"

rm -rf ${CLFS_BUILD_DIR}/$PACKAGE
rm -rf ${CLFS_STATUS_DIR}/$PACKAGE
rm -rf $CLFS_ERROR_FILE
