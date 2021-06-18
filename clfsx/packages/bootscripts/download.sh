#!/bin/bash
## @file bootscripts/download.sh
## @brief download bootscripts and unpack to the build directory
## @details if the zip exists in the sources directory, it will just be used (see common.sh)
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in root directory).

set -e
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh
. ./config.sh

standard_get_and_unzip

CD=`dirname "$0"`
PACKAGE_NAME=`basename $CD`
STEP=`basename $0`
set_status $PACKAGE_NAME $STEP "completed_successfully"