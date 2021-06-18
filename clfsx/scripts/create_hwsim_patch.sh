#!/bin/bash
#
# usage: 
#        modified mac80211_hwsim.c in clfsx/packages
#        original mac80211_hwsim.c in build/build/linux/drivers/net/wireless
#        run this from clfsx/scripts folder
#        new mac80211_hwsim.c.patch in clfsx/patches
#
#set -x
. $CLFSPROJ/config.sh
. $CLFSPROJ/architecture.sh
. $CLFS/scripts/common.sh

mkdir $CLFS_BUILD_DIR/linux/drivers/net/wireless-new
cp $CLFS_PACKAGES_DIR/linux/mac80211_hwsim.c $CLFS_BUILD_DIR/linux/drivers/net/wireless-new/mac80211_hwsim.c
cd $CLFS_BUILD_DIR/linux/drivers/net
diff -rupN wireless/mac80211_hwsim.c wireless-new/mac80211_hwsim.c > mac80211_hwsim.c.patch
mv mac80211_hwsim.c.patch $CLFS_PATCH_DIR
cd $CLFS_SCRIPTS_DIR