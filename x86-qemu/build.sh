#!/bin/bash
## @file x86-qemu/build.sh
## @brief either build from scratch (clean) or build the packages indicated in the input file
## @details the two main things this adds on top of the build_packages.sh script are 
# 1) wiping the build directory and starting from scratch if no input file is given
# 2) copying some things to the build directory so everything needed is right there (nad no other directories need to be referenced)
# 3) copying some things back the Linux boot image so it is avaiable (even if the build direcxtory is deleted)
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# if not given an arg them resets and builds from scratch
# if given an arg then just build the given package file
#set -x
if [ $# -lt 1 ]
then
  cd ..
  rm -rf build
  mkdir build
  cd build
  cp -r ${CLFSPROJ}/* .
  PFILE="packages"
else
    PFILE=$1
    cp -u -r * ../build
    cd ../build
fi

OLD_PROJ_DIR=${CLFSPROJ}
export CLFSPROJ=${PWD}
${CLFS}/scripts/build_packages.sh $PFILE
if [ -f "${CLFSPROJ}/status/this_file_indicates_an_error_occured" ]; then 
  echo "exiting build"
  exit $?
else
  #echo "copying targetfs to $OLD_PROJ_DIR"
  #rm -rf ${OLD_PROJ_DIR}/targetfs
  #cp --copy-contents --parents -RP -u -r targetfs ${OLD_PROJ_DIR}  > /dev/null 2>&1 
  echo "copying bzImage to $OLD_PROJ_DIR"
  cd ${OLD_PROJ_DIR}
  cp ../build/bzImage .
fi
