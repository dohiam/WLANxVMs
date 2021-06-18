#!/bin/bash
## @file gdb/install_targetfs.sh
## @brief builds gdb and copies executables to the targetfs directory
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

# Build gdb
# Note that gdb needs both host and target toolchains
CFLAGS=""
CC_FOR_TARGET=$CC
GCC_FOR_TARGET=$CC
AR_FOR_TARGET=$AR
AS_FOR_TARGET=$AS
LD_FOR_TARGET=$LD
NM_FOR_TARGET=$NM
OBJCOPY_FOR_TARGET=$OBJCOPY
OBJDUMP_FOR_TARGET=$OBJDUMP
RANLIB_FOR_TARGET=$RANLIB
READLELF_FOR_TARGET=$READELF
STRIP_FOR_TARGET=$STRIP
CC="cc"
GCC="gcc"
AR="ar"
AS="as"
LD="ld"
NM="nm"
OBJCOPY="objcopy"
OBJDUMP="objdump"
RANLIB="ranlib"
READLELF="readelf"
STRIP="strip"
export CC_FOR_TARGET AR_FOR_TARGET AS_FOR_TARGET LD_FOR_TARGET NM_FOR_TARGET OBJCOPY_FOR_TARGET OBJDUMP_FOR_TARGET RANLIB_FOR_TARGET READLELF_FOR_TARGET STRIP_FOR_TARGET
export CC GCC AR AS LD NM OBJCOPY OBJDUMP RANLIB READLELF STRIP 
LOG="${CLFS_STATUS_DIR}/${PACKAGE_NAME}/logfile.txt"
touch $LOG
chmod ugo+rw $LOG
echo "configuring gdb"
#mkdir ../gdbserver
#cp -r gdb/gdbserver/* ../gdbserver/
cd ..
mkdir gdb-build
cd gdb-build
#../gdb/gdb/gdbserver/configure --build="${CLFS_HOST}" --host="${CLFS_HOST}" --target="${CLFS_TARGET}" --prefix="${CLFS_BUILD_DIR}/gdb-install"  > $LOG 2>&1
#../gdb/gdb/gdbserver/configure --build="${CLFS_HOST}" --host="${CLFS_HOST}" --target="${CLFS_TARGET}" --prefix="${CLFS_BUILD_DIR}/gdb-install"
../gdb/configure --build="${CLFS_HOST}" --host="${CLFS_HOST}" --target="${CLFS_TARGET}" --prefix="${CLFS_BUILD_DIR}/gdb-install"  > $LOG 2>&1
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi
echo "compiling gdb"
make -j $JOBS  > $LOG 2>&1
#make -j $JOBS
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi
echo "installing gdb"
make install  > $LOG 2>&1
#make install
if [ $? -ne 0 ]; then echo "exiting $PACKAGE_NAME $STEP due to make failure during configure"; exit $?; fi
cp ${CLFS_BUILD_DIR}/gdb-install/bin/gdbserver ${CLFS_TARGETFS_DIR}/bin

set_status $PACKAGE_NAME $STEP "completed_successfully"
rm -f $CLFS_ERROR_FILE