#!/bin/bash
## @file targetfs/copy_my_targetfs.sh
## @brief copies custome root fs files from the project's mytargetfs to the targetfs directory
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

echo "copying $CLFSPROJ/my_targetfs to $CLFS_TARGETFS_DIR"
cp -rp ${CLFSPROJ}/my_targetfs/* ${CLFS_TARGETFS_DIR}
set_status $PACKAGE_NAME $STEP "completed_successfully"
