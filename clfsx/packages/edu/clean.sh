#!/bin/bash
## @file edu/clean.sh
## @brief clean edu files from the build directory
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# Standard cc build setup
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

PACKAGE="edu"

rm -rf ${CLFS_BUILD_DIR}/$PACKAGE
rm -rf ${CLFS_STATUS_DIR}/$PACKAGE
rm -rf $CLFS_ERROR_FILE
