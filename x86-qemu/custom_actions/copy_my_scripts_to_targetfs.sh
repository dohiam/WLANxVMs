#!/bin/bash
## @file custom_actions/copy_my_scripts_to_targetfs.sh
## @brief copies scripts used for testing into the root home folder of targetfs
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

cp ${CLFSPROJ}/start_hwsim ${CLFS_TARGETFS_DIR}/root