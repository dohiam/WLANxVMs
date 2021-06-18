#!/bin/bash
## @file linux/patch_gen_initramfs_list.sh
## @brief installs a small patch for generating the initramfs
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

# Apply patch
cd $CLFS_PATCHING_DIR
cp $CLFS_PATCH_DIR/gen_initramfs_list_patch .
patch -p0 < gen_initramfs_list_patch

if [ $? -ne 0 ]; then echo "exiting due to make failure"; exit $?; fi

set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE