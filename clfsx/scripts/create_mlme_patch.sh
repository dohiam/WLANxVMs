#!/bin/bash
## @file create_mlme_patch.sh
## @brief diffs release mlme.c with the one in the linux package folder to create a patch file
#
#fine-print: Copyright (c) 2020-2021, David Hamilton <david@davidohamilton.com>. This software may be freely copied and used under GPLv2 (see LICENSE.txt in roocrediffs release mlme.c with the one in the linux package folder to create a patch file

set -x
. $CLFSPROJ/config
. $CLFSPROJ/architecture
. $CLFS/scripts/common

mkdir $CLFS_BUILD_DIR/linux/net/mac80211-new
cp $CLFS_PACKAGES_DIR/linux/mlme.c $CLFS_BUILD_DIR/linux/net/mac80211-new/mlme.c
cd $CLFS_BUILD_DIR/linux/net
diff -rupN mac80211/mlme.c mac80211-new/mlme.c > mlme.c.patch
mv mlme.c.patch $CLFS_SOURCE_DIR
cd $CLFS_SCRIPTS_DIR