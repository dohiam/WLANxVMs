#!/bin/bash
## @file scripts/run_minimal.sh
## @brief creates an environment just for cross compilation 
## @details iwithout any environment variables carried over from the regular host environment
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).


. $CLFSPROJ/config.sh
. ${CLFSPROJ}/architecture.sh
. $CLFS/scripts/common.sh

env -i - CLFS=${CLFS} \
         CLFSPROJ=${CLFSPROJ} \
		 XTOOLS=${XTOOLS} \
         CLFS_SOURCE_DIR="${CLFS_SOURCE_DIR}" \
         CLFS_BUILD_DIR="${CLFS_BUILD_DIR}" \
         CLFS_SCRIPTS_DIR="${CLFS_SCRIPTS_DIR}" \
         CLFS_PACKAGES_DIR="${CLFS_PACKAGES_DIR}" \
         CLFS_TARGETFS_DIR="${CLFS_TARGETFS_DIR}" \
		 CLFS_SYSROOT_DIR="${XTOOLS}/${CLFS_HOST}/${CLFS_TARGET}/sysroot" \
         PATH=${XTOOLS}/${CLFS_HOST}/bin:/bin:/usr/bin \
         MAKEL="make ARCH=${CLFS_ARCH} CROSS_COMPILE=${CLFS_TARGET}" \
         CC="${CLFS_TARGET}-gcc --sysroot=${CLFS_SYSROOT_DIR} ${CFLAGS}" \
         AR="${CLFS_TARGET}-ar" \
         AS="${CLFS_TARGET}-as" \
         NM="${CLFS_TARGET}-nm" \
         OBJCOPY="${CLFS_TARGET}-objcopy" \
         OBJDUMP="${CLFS_TARGET}-objdump" \
         LD="${CLFS_TARGET}-ld --sysroot=${CLFS_SYSROOT_DIR}" \
         RANLIB="${CLFS_TARGET}-ranlib" \
         READELF="${CLFS_TARGET}-readelf" \
         STRIP="${CLFS_TARGET}-strip" \
		 TERM="${TERM}" \
		 CFLAGS="${CFLAGS}" \
   bash -noprofile  -l -c "$1"
