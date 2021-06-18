#!/bin/bash
## @file iana-etc/install_targetfs.sh
## @brief builds iana-etc and copies executables to the targetfs directory
## @details if there is any error then leave the error_file to indicate it
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# Standard cc build setup
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

#set +x

touch ${CLFS}/.error

CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`
cd $CLFS_BUILD_DIR/$PACKAGE_NAME
# Build init-etc
# Note that patch is applied when downloaded and unzipped
LOG="${CLFS_STATUS_DIR}/${PACKAGE_NAME}/logfile.txt"
touch $LOG
chmod ugo+rw $LOG
echo "installing iana-etc"
if [ -f $CLFS_SOURCE_DIR/protocol-numbers.iana ]; then
  cp $CLFS_SOURCE_DIR/protocol-numbers.iana $CLFS_BUILD_DIR/$PACKAGE_NAME
  cp $CLFS_SOURCE_DIR/port-numbers.iana $CLFS_BUILD_DIR/$PACKAGE_NAME
else
  make get  > $LOG 2>&1
  if [ $? -ne 0 ]; then echo "exiting due to make get failure"; exit $?; fi
fi
make STRIP=yes  > $LOG 2>&1
make DESTDIR=${CLFS_TARGETFS_DIR} install
if [ $? -ne 0 ]; then echo "exiting due to make install strip failure"; exit $?; fi

set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE