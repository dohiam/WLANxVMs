#!/bin/bash
## @file linux/download.sh
## @brief download linux, unpack to the build directory, and patch mac80211 files as needed. See patches in the patches folder.
## @details if the zip exists in the sources directory, it will just be used (see common.sh) without (re) patching
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

set +x
# if argument passed then recipe is run for a specific purpose so skip out
if [[ $# -gt 0 ]]; then exit 0; fi

set -e
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh
. ./config.sh

PATCH_FILE_1="mlme.c.patch"
ORIGINAL_FILE_1="mlme.c"
SUBDIR_1="linux/net/mac80211"

PATCH_FILE_2="mac80211_hwsim.c.patch"
ORIGINAL_FILE_2="mac80211_hwsim.c"
SUBDIR_2="linux/drivers/net/wireless"

if [ -d $CLFS_BUILD_DIR/$SUBDIR_1 ]; then
   echo "build directory found; assuming already patched"
   need_patch=false
else
   need_patch=true
fi
standard_get_and_unzip

if ($need_patch); then
  cd ${CLFS_PATCH_DIR}
  cp $PATCH_FILE_1 $CLFS_BUILD_DIR/$SUBDIR_1
  cd $CLFS_BUILD_DIR/$SUBDIR_1
  patch -p1 -i $PATCH_FILE_1
  echo "applied $PATCH_FILE_1" 
  cd ${CLFS_PATCH_DIR}
  cp $PATCH_FILE_2 $CLFS_BUILD_DIR/$SUBDIR_2
  cd $CLFS_BUILD_DIR/$SUBDIR_2
  patch -p1 -i $PATCH_FILE_2
  echo "applied $PATCH_FILE_2" 
fi  

CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`
set_status $PACKAGE_NAME $STEP "completed_successfully"