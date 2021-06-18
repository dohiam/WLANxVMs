#!/bin/bash
## @file targetfs/other_actions.sh
## @brief TBD
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

# if argument not passed then recipe is not run for a specific purpose 
if [[ $# -eq 0 ]]; then exit 0; fi

# Standard cc build setup
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

touch $CLFS_ERROR_FILE

CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`

# do specific action
. $CD/$1.sh

rm -f $CLFS_ERROR_FILE