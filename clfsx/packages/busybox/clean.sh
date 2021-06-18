#!/bin/bash
## @file busybox/clean.sh
## @brief clean bosybox files from the build directory
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# Standard cc build setup
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

# clean busybox
rm -rf ${CLFS_BUILD_DIR}/busybox
rm -rf ${CLFS_STATUS_DIR}/busybox
