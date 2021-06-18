#!/bin/bash
## @file wpa_supplicant/install_targetfs.sh
## @brief builds wpa supplicant and copies executables to the targetfs directory
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

# Build wpa_supplicant
cd wpa_supplicant
export PKG_CONFIG_PATH+=${CLFS_BUILD_DIR}/libnl-install/lib/pkgconfig
export DESTDIR=${CLFS_TARGETFS_DIR}
export BINDIR=/bin
echo "using wpa_supplicant.config"
cp ${CLFSPROJ}/hostapd.config .config
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to missing config file"; exit $?; fi
LOG="${CLFS_STATUS_DIR}/${PACKAGE_NAME}/logfile.txt"
touch $LOG
chmod ugo+rw $LOG
echo "compiling wpa_supplicant"
make -j $JOBS   > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi
echo "installing wpa_supplicant"
make install   > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi

set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE