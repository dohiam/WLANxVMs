#!/bin/bash
## @file custom_actions/include_headers_from_xtools.sh
## @brief builds bootscripts and copies executables to the targetfs directory
## @details if there is any error then leave the error_file to indicate it
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

# add include directory from xtools sysroot into CCFLAGS
add_include_path "${CLFS_CROSSTOOLS_DIR}/${CLFS_HOST}/${CLFS_TARGET}/sysroot/usr/include"
