#!/bin/bash
## @file clfs_system/project_folders.sh
## @brief creates all the various project directories if needed
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`
DONE=$(check_status $PACKAGE_NAME $STEP)
mkdir -p $CLFS_BUILD_DIR
mkdir -p $CLFS_TARGETFS_DIR
mkdir -p $CLFS_STATUS_DIR
mkdir -p $CLFS_PATCHING_DIR
set_status $PACKAGE_NAME $STEP "completed_successfully"
