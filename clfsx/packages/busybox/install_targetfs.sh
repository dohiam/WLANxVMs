#!/bin/bash
## @file busybox/install_targetfs.sh
## @brief builds busybox and copies executables to the targetfs directory
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

# Build busybox to run on target
make distclean
if [ $? -ne 0 ]; then echo "exiting due to make failure"; exit $?; fi
if [ -f ${CLFSPROJ}/busybox.config ]; then
  echo "using existing busybox.config"
  cp ${CLFSPROJ}/busybox.config .config
else
  make ARCH="${CLFS_ARCH}" defconfig
fi
if [ $? -ne 0 ]; then echo "exiting due to make failure"; exit $?; fi
if ($MUSL); then
  # Disable building both ifplugd and inetd as they both have issues building against musl
  sed -i 's/\(CONFIG_\)\(.*\)\(INETD\)\(.*\)=y/# \1\2\3\4 is not set/g' .config
  sed -i 's/\(CONFIG_IFPLUGD\)=y/# \1 is not set/' .config
  # Disable the use of utmp/wtmp as musl does not support them
  sed -i 's/\(CONFIG_FEATURE_WTMP\)=y/# \1 is not set/' .config
  sed -i 's/\(CONFIG_FEATURE_UTMP\)=y/# \1 is not set/' .config
  # Disable the use of ipsvd for both TCP and UDP as it has issues building against musl (similar to inetd's issue
  sed -i 's/\(CONFIG_UDPSVD\)=y/# \1 is not set/' .config
  sed -i 's/\(CONFIG_TCPSVD\)=y/# \1 is not set/' .config
fi
# make
LOG="${CLFS_STATUS_DIR}/${PACKAGE_NAME}/logfile.txt"
touch $LOG
chmod ugo+rw $LOG
echo "compiling busybox"
make CFLAGS=$CFLAGS -j $JOBS ARCH="${CLFS_ARCH}" CROSS_COMPILE="${CLFS_TARGET}-" > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting due to make failure; see $LOG"; exit $?; fi
echo "installing busybox"
make CFLAGS=$CFLAGS -j $JOBS ARCH="${CLFS_ARCH}" CROSS_COMPILE="${CLFS_TARGET}-" CONFIG_PREFIX="${CLFS_TARGETFS_DIR}" install  > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting due to make failure; see $LOG"; exit $?; fi
echo "done installing busybox"
# For modules
#if ($USING_MODULES); then
#  echo "installing example/depmod"
#  cp -v examples/depmod ${CLFS_TARGETFS_DIR}/bin
#  chmod -v 755 ${CLFS_TARGETFS_DIR}/bin/depmod.pl
#fi
set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE