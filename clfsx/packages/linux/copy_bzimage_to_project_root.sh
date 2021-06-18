#!/bin/bash
## @file linux/copy_bzimage_to_project_root.sh
## @brief bjust copies the bootable image to the project directory for use
## @details if there is any error then leave the error_file to indicate it
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

touch $CLFS_ERROR_FILE

echo "Copying bzImage to ${CLFSPROJ}"
cp -L arch/${CLFS_ARCH}/boot/bzImage ${CLFSPROJ}/bzImage

rm -f $CLFS_ERROR_FILE