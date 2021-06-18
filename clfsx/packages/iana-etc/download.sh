#!/bin/bash
## @file iana-etc/download.sh
## @brief download iana-etc and unpack to the build directory
## @details if the zip exists in the sources directory, it will just be used (see common.sh)
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

#set +x
# if argument passed then recipe is run for a specific purpose so skip out
if [[ $# -gt 0 ]]; then exit 0; fi

set -e
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh
. ./config.sh

if [ -d $CLFS_BUILD_DIR/$PACKAGE_NAME ]; then
   echo "iana-etc build directory found; assuming already patched"
   need_patch=false
else
   need_patch=true
fi
standard_get_and_unzip
if ($need_patch); then
  cd ${CLFS_PATCH_DIR}
  if [ ! -f $CLFS_PATCH_DIR/iana-etc-2.30-update-2.patch ]; then 
    wget -c $PATCH_URL
  fi
  cp $PATCH_FILE $CLFS_BUILD_DIR/$PACKAGE_NAME
  cd $CLFS_BUILD_DIR/$PACKAGE_NAME
  patch -Np1 -i ./iana-etc-2.30-update-2.patch
fi  

CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`
set_status $PACKAGE_NAME $STEP "completed_successfully"