#!/bin/bash
## @file clfs_system/clean.sh
## @brief clean bootscripts files from the build directory
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# Standard build setup

. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

if [ ! -d $CLFS_STATUS_DIR ]; then mkdir -p $CLFS_STATUS_DIR; fi
clear_include_path
rm -rf $CLFS_STATUS_DIR
rm -rf $CLFS_BUILD_DIR
rm -rf $CLFS_TARGETFS_DIR
