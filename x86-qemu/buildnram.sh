#!/bin/bash
## @file x86-qemu/buildnram.sh
## @brief same as build.sh but for a ramdisk
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# if not given an arg them resets and builds from scratch
# if given an arg then just build the given package file
#set -x
rm -rf ${CLFSPROJ}/targetfs
if [ $# -lt 1 ]
then
  sudo umount myramdisk > /dev/null 2>&1 
  cd ..
  sudo mount -t tmpfs -o size=4G myramdisk ramdisk
  cd ramdisk
  cp -r ${CLFSPROJ}/* .
  PFILE="packages"
else
    PFILE=$1
    cp -u -r * ../ramdisk
    cd ../ramdisk
fi

OLD_PROJ_DIR=${CLFSPROJ}
export CLFSPROJ=${PWD}
${CLFS}/scripts/build_packages $PFILE
if [ -f "${CLFSPROJ}/status/this_file_indicates_an_error_occured" ]; then 
  echo "exiting build"
  exit $?
else
  echo "copying targetfs to $OLD_PROJ_DIR"
  cp --copy-contents --parents -RP -u -r targetfs ${OLD_PROJ_DIR}  > /dev/null 2>&1 
  echo "copying bzImage to $OLD_PROJ_DIR"
  cd ${OLD_PROJ_DIR}
  cp ../ramdisk/bzImage .
fi
