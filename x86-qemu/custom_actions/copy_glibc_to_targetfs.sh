#!/bin/bash
## @file custom_actions/copy_glibc_to_targetfs.sh
## @brief copies all the libraries used from x-tools to the targetfs
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).


. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

echo "copying glibc from $XTOOLS"
lib_src=${XTOOLS}/${CLFS_HOST}/${CLFS_TARGET}/sysroot/lib
lib_dst=${CLFSPROJ}/targetfs/lib
rm -rf ${lib_dst}/ld-2.29.so
rm -rf ${lib_dst}/libc-2.29.so
rm -rf ${lib_dst}/libm-2.29.so
rm -rf ${lib_dst}/librt-2.29.so
rm -rf ${lib_dst}/libdl-2.29.so
rm -rf ${lib_dst}/libpthread-2.29.so
rm -rf ${lib_dst}/ld-linux-x86-64.so.2
rm -rf ${lib_dst}/libm.so.6
rm -rf ${lib_dst}/libc.so.6
rm -rf ${lib_dst}/librt.so.1
rm -rf ${lib_dst}/libpthread.so.0
rm -rf ${lib_dst}/../lib64
cp ${lib_src}/ld-2.29.so         ${lib_dst}
cp ${lib_src}/libc-2.29.so       ${lib_dst}
cp ${lib_src}/libm-2.29.so       ${lib_dst}
cp ${lib_src}/librt-2.29.so      ${lib_dst}
cp ${lib_src}/libdl-2.29.so      ${lib_dst}
cp ${lib_src}/libpthread-2.29.so ${lib_dst}
cd ${lib_dst}
ln -s ld-2.29.so         ld-linux-x86-64.so.2
ln -s libm-2.29.so       libm.so.6
ln -s libc-2.29.so       libc.so.6
ln -s librt-2.29.so      librt.so.1
ln -s libdl-2.29.so      libdl.so.2
ln -s libpthread-2.29.so libpthread.so.0
cd ..
ln -s lib lib64